/*
 * This software implements an IEC 60870-5-104 protocol tester.
 * Copyright ?2010,2011,2012 Ricardo L. Olsen
 *
 * Disclaimer
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <QDir>
#include <QCloseEvent>
#include <string>
#include <time.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"

//-------------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    BDTR_Logar = 1;
    i104.mLog.deactivateLog();

    // busca configuracoes no arquivo ini
    QSettings settings( "./qtester104.ini", QSettings::IniFormat );

    i104.setPrimaryAddress( settings.value( "IEC104/PRIMARY_ADDRESS", 1 ).toInt() );
    i104.setSecondaryAddress( settings.value( "RTU1/SECONDARY_ADDRESS", 1 ).toInt() );
    i104.SendCommands = settings.value( "RTU1/ALLOW_COMMANDS", 0 ).toInt();

    QString IPEscravo;
    IPEscravo = settings.value( "RTU1/IP_ADDRESS", "" ).toString();
    i104.setSecondaryIP ( (char *)IPEscravo.toStdString().c_str() );
    i104.setPortTCP( settings.value( "RTU1/TCP_PORT", i104.getPortTCP() ).toInt() );

    // this is for using with the OSHMI HMI in a dual architecture
    QSettings settings_bdtr( "./ihm.ini", QSettings::IniFormat );
    BDTR_host_dual = settings_bdtr.value( "REDUNDANCIA/IP_OUTRO_IHM", "" ).toString();
    BDTR_host = "127.0.0.1";
    BDTR_CntDnToBePrimary = BDTR_CntToBePrimary;

    ui->setupUi( this );

    // this is for hiding the window when runnig
    Hide = settings_bdtr.value( "RUN/HIDE", "" ).toInt();

    this->setWindowTitle( tr("QTester104 IEC60870-5-104") );

    QIntValidator * validator = new QIntValidator( 0, 255, this );
    ui->leLinkAddress->setValidator( validator );
    ui->leMasterAddress->setValidator( validator );

    QRegExp rx( "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b" );
    QValidator *valip = new QRegExpValidator( rx, this );
    ui->leIPRemoto->setValidator( valip );

    udps = new QUdpSocket();
    udps->bind( BDTR_porta_escuta );
    udps->open( QIODevice::ReadWrite );

    QString qs;
    ui->leIPRemoto->setText( IPEscravo );
    QTextStream( &qs ) << i104.getSecondaryAddress();
    ui->leLinkAddress->setText( qs );
    qs = "";
    QTextStream( &qs ) << i104.getPrimaryAddress();
    ui->leMasterAddress->setText( qs );

    ui->leIPRemoto->setText( IPEscravo );

    tmLogMsg = new QTimer();
    tmBDTR_kamsg = new QTimer();

    connect( udps, SIGNAL(readyRead()), this, SLOT(slot_BDTR_pronto_para_ler()) );
    connect( tmLogMsg, SIGNAL(timeout()), this, SLOT(slot_timer_logmsg()) );
    connect( tmBDTR_kamsg, SIGNAL(timeout()), this, SLOT(slot_timer_BDTR_kamsg()) );
    connect( &i104, SIGNAL(signal_dataIndication(iec_obj *, int)), this, SLOT(slot_dataIndication(iec_obj *, int)) );
    connect( &i104, SIGNAL(signal_interrogationActConfIndication()), this, SLOT(slot_interrogationActConfIndication()) );
    connect( &i104, SIGNAL(signal_interrogationActTermIndication()), this, SLOT(slot_interrogationActTermIndication()) );
    connect( &i104, SIGNAL(signal_tcp_connect()), this, SLOT(slot_tcpconnect()) );
    connect( &i104, SIGNAL(signal_tcp_disconnect()), this, SLOT(slot_tcpdisconnect()) );
    connect( &i104, SIGNAL(signal_commandActConfIndication(iec_obj *)), this, SLOT(slot_commandActConfIndication(iec_obj *)) );
    connect( &i104, SIGNAL(signal_commandActTermIndication(iec_obj *)), this, SLOT(slot_commandActTermIndication(iec_obj *)) );

    ui->pbGI->setEnabled(false);
    ui->pbSendCommandsButton->setEnabled( false );

    ui->twPontos->clear();
    ui->twPontos->setSortingEnabled ( false );
    ui->twPontos->setColumnCount( 6 );
    ui->twPontos->sortByColumn( 0 );

    if ( IPEscravo != "" )
      on_pbConnect_clicked();

    QStringList colunas;
    colunas << "Address" << "Value" << "Type" << "Cause" << "Flags" << "Count";
    ui->twPontos->setHorizontalHeaderLabels( colunas );

    tmLogMsg->start(1000);

    if ( BDTR_HaveDualHost() )
    {
        tmBDTR_kamsg->start( BDTR_seconds_kamsg * 1000 );
        isPrimary = false;
        i104.disable_connect();
        ui->lbMode->setText( "<font color='red'>Secondary</font>" );
    }
    else
    {
        isPrimary = true;
        i104.enable_connect();
        ui->lbMode->setText( "<font color='green'>Primary</font>" );
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tmLogMsg;
    delete tmBDTR_kamsg;
}

void MainWindow::on_pbGI_clicked()
{
    i104.solicitGI();
}

void MainWindow::on_pbConnect_clicked()
{
    if ( i104.tmKeepAlive->isActive() )
    {
        i104.tmKeepAlive->stop();
        i104.tcps->close();
        i104.slot_tcpdisconnect();
    }
    else
    {
        i104.setSecondaryIP( (char*)ui->leIPRemoto->text().toStdString().c_str() );
        i104.setSecondaryAddress( ui->leLinkAddress->text().toInt() );
        i104.setPrimaryAddress( ui->leMasterAddress->text().toInt() );

        QString qs;
        ui->leIPRemoto->setText( i104.getSecondaryIP() );
        QTextStream( &qs ) <<  ui->leLinkAddress->text().toInt();
        ui->leLinkAddress->setText( qs );
        qs="";
        QTextStream( &qs ) << ui->leMasterAddress->text().toInt();
        ui->leMasterAddress->setText( qs );

        ui->leIPRemoto->setEnabled( false );
        ui->leLinkAddress->setEnabled( false );
        ui->leMasterAddress->setEnabled( false );

        ui->pbConnect->setText( "Give up..." );
        ui->lbStatus->setText( "<font color='green'>TRYING TO CONNECT!</font>" );

        mapPtItem_ColAddress.clear();
        mapPtItem_ColValue.clear();
        mapPtItem_ColType.clear();
        mapPtItem_ColCause.clear();
        mapPtItem_ColFlags.clear();
        mapPtItem_ColCount.clear();
        ui->twPontos->clearContents();
        ui->twPontos->setRowCount ( 0 );
        ui->lwLog->clear();

        i104.tmKeepAlive->start(1000);
    }
}

// recebimento de informacoes pelo BDTR
void MainWindow::slot_BDTR_pronto_para_ler()
{
    char buf[5000];
    char bufOut[1600];  // buffer para mensagem bdtr
    buf[0]=0;

    unsigned char br[2000]; // buffer de recepcao
    int bytesrec;

    QHostAddress address;
    quint16 port;
    bytesrec = udps->readDatagram ( (char *)br, sizeof(br), &address, &port );

    sprintf( buf+strlen(buf), "%3d: ", bytesrec );
    for ( int i=0; i< bytesrec; i++ )
        sprintf (buf+strlen(buf), "%02x ", br[i]);
    BDTR_Loga( buf );

    int Tipo = br[0] & T_MASC;

    switch ( Tipo )
    {
    case T_REQ:
        {
            msg_req * msg;
            msg = (msg_req*)br;

            if ( msg->TIPO == REQ_GRUPO && msg->ID == 0 ) // GI
            {
                BDTR_Loga( "--> BDTR: REQ GI" );
                i104.solicitGI();
            }
            if ( msg->TIPO == REQ_GRUPO && msg->ID == 255 ) // request group 255: show form
            {
                Hide = ! Hide;
                if ( ! Hide )
                  this->setVisible( true );
            }
            else
            {
                if ( msg->TIPO == REQ_HORA )
                  BDTR_Loga( "--> BDTR: IGNORED (TIME REQ)" );
                else
                  BDTR_Loga( "--> BDTR: IGNORED (REQ ?)" );
            }
        }
        break;
    case T_HORA:
        if ( address == BDTR_host_dual )
          {
          if ( isPrimary )
            {
              BDTR_Loga( "--> BDTR: KEEPALIVE RECEIVED FROM DUAL MACHINE(TIME), BECOMING SECONDARY" );
              ui->lbMode->setText( "<font color='red'>Secondary</font>" );
            }
          isPrimary = false;
          i104.disable_connect();
          BDTR_CntDnToBePrimary = BDTR_CntToBePrimary; // restart count to be primary
          }
        else
          BDTR_Loga( "--> BDTR: IGNORED (TIME)" );
        break;
    case T_COM: // COMANDO
        {
            msg_com * msg;
            msg = (msg_com*)br;
            bool enviar = false;

            BDTR_Loga("--> BDTR COM");

            if ( msg->TVAL == T_DIG ) // DIGITAL
            {
                msg_ack *ms;
                ms = (msg_ack*)bufOut;
                // status bits 11 and 00 are used for command blocking
                // forward only commands for status = 10 (ON) or = 01 (OFF)
                if ( (msg->PONTO.STATUS & ESTADO) != 3 && (msg->PONTO.STATUS & ESTADO) != 0 )
                {
                    iec_obj obj;
                    obj.cause = iec104_class::ACTIVATION;
                    obj.address = msg->PONTO.ID;
                    obj.ca = msg->PONTO.VALOR.COM_SEMBANCO.UTR;
                    obj.qu = msg->PONTO.VALOR.COM_SEMBANCO.COMIEC.qu;
                    obj.se = msg->PONTO.VALOR.COM_SEMBANCO.COMIEC.se;

                    switch ( msg->PONTO.VALOR.COM_SEMBANCO.ASDU )
                      {
                      case 0: // if ASDU not defined, use single command
                        msg->PONTO.VALOR.COM_SEMBANCO.ASDU = iec104_class::C_SC_NA_1;
                      case iec104_class::C_SC_NA_1:
                      case iec104_class::C_SC_TA_1: // single
                        obj.type = msg->PONTO.VALOR.COM_SEMBANCO.ASDU;
                        obj.scs = !(msg->PONTO.VALOR.COM_SEMBANCO.COMIEC.dcs & 0x01);
                        enviar=true;
                        break;

                      case iec104_class::C_DC_NA_1:
                      case iec104_class::C_DC_TA_1: // double
                        obj.type = msg->PONTO.VALOR.COM_SEMBANCO.ASDU;
                        obj.dcs = msg->PONTO.VALOR.COM_SEMBANCO.COMIEC.dcs;
                        enviar=true;
                        break;

                      case iec104_class::C_RC_NA_1:
                      case iec104_class::C_RC_TA_1: // reg. step
                        obj.type = msg->PONTO.VALOR.COM_SEMBANCO.ASDU;
                        obj.rcs = msg->PONTO.VALOR.COM_SEMBANCO.COMIEC.dcs;
                        enviar=true;
                        break;

                      default:
                        enviar=false;
                        break;
                      }

                 if (enviar)
                    {
                    // forward command to IEC104
                    i104.sendCommand( &obj );
                    LastCommandAddress = obj.address;
                    // Vai enviar ack pelo BDTR ao receber o activation em n�vel de 104
                    }
                else
                    { // REJECT COMMAND (ASDU not supported)
                    ms->COD = T_ACK;
                    ms->TIPO = T_COM;
                    ms->ORIG = BDTR_orig;
                    ms->ID = 0x80 | msg->PONTO.VALOR.COM_SEMBANCO.COMIEC.dcs;
                    ms->COMP = msg->PONTO.ID;
                    udps->writeDatagram ( (const char *) bufOut, sizeof( msg_ack ), BDTR_host, BDTR_porta );
                    if ( BDTR_HaveDualHost() )
                      udps->writeDatagram ( (const char *) bufOut, sizeof( msg_ack ), BDTR_host_dual, BDTR_porta );

                    BDTR_Loga( "<-- BDTR: COMMAND REJECTED, UNSUPPORTED ASDU" );
                    }
                }
            }
        }
        break;
    case T_DIG:
        BDTR_Loga( "--> BDTR: IGNORED MSG (DIGITAL)" );
        break;
    default:
        BDTR_Loga( "--> BDTR: IGNORED MSG" );
        break;
    }
}

void MainWindow::BDTR_processPoints( iec_obj *obj, int numpoints )
{

    TFA_Qual qfa;
    int tam_msg;

    // Aten��o, vou deixar o bit T_CONV do c�digo da mensagem para sinalizar varredor sem banco

    switch ( obj->type )
      {
      case iec104_class::M_DP_TB_1: // duplo com tag
      case iec104_class::M_SP_TB_1: // simples com tag
        {
        msg_dig_tag *msgdigtag;

        tam_msg = sizeof( A_dig_tag ) * numpoints + sizeof( msg_dig_tag ) - sizeof( A_dig_tag );
        msgdigtag = (msg_dig_tag*)malloc( tam_msg );
        msgdigtag->COD = T_DIG_TAG;
        if ( obj->cause == iec104_class::CYCLIC )
          msgdigtag->COD |= T_CIC;
        if ( obj->cause == iec104_class::SPONTANEOUS )
          msgdigtag->COD |= T_SPONT;
        msgdigtag->NRPT = numpoints;
        msgdigtag->ORIG = BDTR_orig;

        for ( int cntpnt=0; cntpnt < numpoints; cntpnt++, obj++ )
          {
          // converte o qualificador do IEC para formato A do PABD
          qfa.Byte = 0;
          qfa.Subst = obj->bl || obj->sb;
          qfa.Tipo = TFA_TIPODIG;
          qfa.Falha = obj->iv || obj->nt;
          qfa.FalhaTag = obj->timetag.iv;

          if ( obj->type == iec104_class::M_DP_TB_1 || obj->type == iec104_class::M_DP_NA_1 )
            {
            qfa.Duplo = obj->dp;
            }
          else
            { // simples para duplo
            qfa.Estado = !obj->sp;
            qfa.EstadoH = obj->sp;
            }

          msgdigtag->PONTO[cntpnt].ID = obj->address;
          msgdigtag->PONTO[cntpnt].UTR = obj->ca;
          msgdigtag->PONTO[cntpnt].STAT = qfa.Byte;
          msgdigtag->PONTO[cntpnt].TAG.ANO = 2000+obj->timetag.year;
          msgdigtag->PONTO[cntpnt].TAG.MES = obj->timetag.month;
          msgdigtag->PONTO[cntpnt].TAG.DIA = obj->timetag.mday;
          msgdigtag->PONTO[cntpnt].TAG.HORA = obj->timetag.hour;
          msgdigtag->PONTO[cntpnt].TAG.MINUTO = obj->timetag.min;
          msgdigtag->PONTO[cntpnt].TAG.MSEGS = obj->timetag.msec;
          }

        udps->writeDatagram ( (const char *) msgdigtag, tam_msg, BDTR_host, BDTR_porta );
        if ( BDTR_HaveDualHost() )
          udps->writeDatagram ( (const char *) msgdigtag, tam_msg, BDTR_host_dual, BDTR_porta );

        free( msgdigtag );
        }
        break;

      case iec104_class::M_DP_NA_1: // duplo sem tag
      case iec104_class::M_SP_NA_1: // simples sem tag
        {
        msg_dig *msgdig;
        tam_msg = sizeof(A_dig) * numpoints + sizeof( msg_dig ) - sizeof( A_dig );
        msgdig = (msg_dig*)malloc( tam_msg );
        msgdig->COD = T_DIG;
        if ( obj->cause == iec104_class::CYCLIC )
          msgdig->COD |= T_CIC;
        if ( obj->cause == iec104_class::SPONTANEOUS )
          msgdig->COD |= T_SPONT;
        msgdig->NRPT = numpoints;
        msgdig->ORIG = BDTR_orig;

        for (int cntpnt=0; cntpnt < numpoints; cntpnt++, obj++)
          {
          // converte o qualificador do IEC para formato A do PABD
          qfa.Byte = 0;
          qfa.Subst = obj->bl || obj->sb;
          qfa.Tipo = TFA_TIPODIG;
          qfa.Falha = obj->iv || obj->nt;
          if ( obj->type == iec104_class::M_DP_TB_1 || obj->type == iec104_class::M_DP_NA_1 )
            {
            qfa.Duplo = obj->dp;
            }
          else
            { // simples para duplo
            qfa.Estado = !obj->sp;
            qfa.EstadoH = obj->sp;
            }

          msgdig->PONTO[cntpnt].ID = obj->address;
          msgdig->PONTO[cntpnt].STAT = qfa.Byte;
          }

        udps->writeDatagram ( (const char *) msgdig, tam_msg, BDTR_host, BDTR_porta );
        if ( BDTR_HaveDualHost() )
            udps->writeDatagram ( (const char *) msgdig, tam_msg, BDTR_host_dual, BDTR_porta );
        free(msgdig);
        }
        break;

      case iec104_class::M_ST_NA_1: // tap
      case iec104_class::M_ME_NA_1: // 9
      case iec104_class::M_ME_NB_1: // 11
        {
        msg_ana *msgana;

        tam_msg = sizeof( A_ana ) * numpoints + sizeof( msg_ana ) - sizeof( A_ana );
        msgana = (msg_ana*) malloc( tam_msg );
        if ( obj->type == iec104_class::M_ME_NA_1 )
          msgana->COD = T_NORM;
        else
          msgana->COD = T_ANA;
        if ( obj->cause == iec104_class::CYCLIC )
          msgana->COD |= T_CIC;
        if ( obj->cause == iec104_class::SPONTANEOUS )
          msgana->COD |= T_SPONT;
        msgana->NRPT = numpoints;
        msgana->ORIG = BDTR_orig;

        for ( int cntpnt=0; cntpnt < numpoints; cntpnt++, obj++ )
          {
          // converte o qualificador do IEC para formato A do PABD
          qfa.Byte = 0;
          qfa.Subst = obj->bl || obj->sb;
          qfa.Tipo = TFA_TIPOANA;
          qfa.Falha = obj->iv || obj->nt || obj->ov;
          if (obj->type == iec104_class::M_ST_NA_1) // tap
             qfa.Falha =  qfa.Falha || obj->t; // transient = falha

          msgana->PONTO[cntpnt].ID = obj->address;
          msgana->PONTO[cntpnt].STAT = qfa.Byte;
          msgana->PONTO[cntpnt].VALOR = obj->value;
          }

        udps->writeDatagram ( (const char *) msgana, tam_msg, BDTR_host, BDTR_porta );
        if ( BDTR_HaveDualHost() )
           udps->writeDatagram ( (const char *) msgana, tam_msg, BDTR_host_dual, BDTR_porta );
        free(msgana);
        }
        break;

      case iec104_class::M_ME_NC_1: // 13
        {
        msg_float *msgflt;

        tam_msg = sizeof(A_float) * numpoints + sizeof(msg_float) - sizeof(A_float);
        msgflt = (msg_float*)malloc( tam_msg );
        msgflt->COD = T_FLT;
        if ( obj->cause == iec104_class::CYCLIC )
          msgflt->COD |= T_CIC;
        if ( obj->cause == iec104_class::SPONTANEOUS )
          msgflt->COD |= T_SPONT;

        msgflt->NRPT = numpoints;
        msgflt->ORIG = BDTR_orig;

        for ( int cntpnt=0; cntpnt < numpoints; cntpnt++, obj++ )
          {
          // qualifier converte o qualificador do IEC para formato A do PABD/BDTR
          qfa.Byte = 0;
          qfa.Subst = obj->bl || obj->sb;
          qfa.Tipo = TFA_TIPOANA;
          qfa.Falha = obj->iv || obj->nt || obj->ov;

          msgflt->PONTO[cntpnt].ID = obj->address;
          msgflt->PONTO[cntpnt].STAT = qfa.Byte;
          msgflt->PONTO[cntpnt].VALOR = obj->value;
          }

        udps->writeDatagram ( (const char *) msgflt, tam_msg, BDTR_host, BDTR_porta );
        if ( BDTR_HaveDualHost() )
            udps->writeDatagram ( (const char *) msgflt, tam_msg, BDTR_host_dual, BDTR_porta );
        free( msgflt );
        }
        break;

      default:
         i104.mLog.pushMsg( "--> IEC104 UNSUPPORTED TYPE, NOT FORWARDED TO BDTR" );
         break;
      }
}

// Envio de comando
void MainWindow::on_pbSendCommandsButton_clicked()
{
    iec_obj obj;

    if ( ui->leCmdValue->text().trimmed() == "" || ui->leCmdAddress->text().trimmed() == "" )
        return;
    if ( ui->leCmdAddress->text().toInt() == 0 )
        return;

    obj.address = ui->leCmdAddress->text().toInt();
    obj.type = ui->cbCmdAsdu->currentText().left(2).toInt();
    obj.value = ui->leCmdValue->text().toInt();
    switch ( obj.type )
    {
    case iec104_class::C_SC_NA_1:
    case iec104_class::C_SC_TA_1:
        obj.scs = ui->leCmdValue->text().toInt();
        break;
    case iec104_class::C_DC_NA_1:
    case iec104_class::C_DC_TA_1:
        obj.dcs = ui->leCmdValue->text().toInt();
        break;
    case iec104_class::C_RC_NA_1:
    case iec104_class::C_RC_TA_1:
        obj.rcs = ui->leCmdValue->text().toInt();
        break;
    }
    obj.qu = ui->cbCmdDuration->currentText().left(1).toInt();
    obj.se = (int)ui->cbSBO->isChecked();

    i104.sendCommand( &obj );
    LastCommandAddress = obj.address;
}

void MainWindow::BDTR_Loga( QString str, int id )
{
    if  (BDTR_Logar && id == 0 )
    {
        i104.mLog.pushMsg( (char*) str.toStdString().c_str(), 0 );
        if ( ui->cbAutoScroll->isChecked() )
          ui->lwLog->scrollToBottom();
    }
}

void MainWindow::slot_dataIndication( iec_obj *obj, int numpoints )
{
    char buf[1000];
    int rw = -1;
    bool inserted = false;
    QTableWidgetItem *pitem;
    static const char* dblmsg[] = { "tra ","off ","on ","ind " };

    BDTR_processPoints( obj, numpoints );

    for (int i=0; i< numpoints; i++, obj++)
    {
        sprintf( buf, "%05u", obj->address );

        pitem = NULL;
        pitem = mapPtItem_ColAddress[obj->address];
        if ( pitem == NULL )
        {
                // insere
                rw = ui->twPontos->rowCount();
                ui->twPontos->insertRow( rw );
                QTableWidgetItem *newItem = new QTableWidgetItem( buf );
                ui->twPontos->setItem( rw, 0, newItem );
                newItem->setFlags( Qt::ItemIsSelectable );
                mapPtItem_ColAddress[obj->address] = newItem;

                newItem = new QTableWidgetItem( buf );
                ui->twPontos->setItem( rw, 1, newItem );
                newItem->setFlags( Qt::ItemIsSelectable );
                mapPtItem_ColValue[obj->address] = newItem;

                newItem = new QTableWidgetItem( buf );
                ui->twPontos->setItem( rw, 2, newItem );
                newItem->setFlags( Qt::ItemIsSelectable );
                mapPtItem_ColType[obj->address] = newItem;

                newItem = new QTableWidgetItem( buf );
                ui->twPontos->setItem( rw, 3, newItem );
                newItem->setFlags( Qt::ItemIsSelectable );
                mapPtItem_ColCause[obj->address] = newItem;

                newItem = new QTableWidgetItem( buf );
                ui->twPontos->setItem( rw, 4, newItem );
                newItem->setFlags( Qt::ItemIsSelectable );
                mapPtItem_ColFlags[obj->address] = newItem;

                newItem = new QTableWidgetItem( buf );
                ui->twPontos->setItem( rw, 5, newItem );
                newItem->setFlags( Qt::ItemIsSelectable );
                newItem->setText( "0" );
                mapPtItem_ColCount[obj->address] = newItem;

                inserted = true;
        }

        sprintf( buf, "%f", obj->value );
        mapPtItem_ColValue[obj->address]->setText( buf );
        sprintf( buf, "%d", obj->type );
        mapPtItem_ColType[obj->address]->setText( buf );
        sprintf( buf, "%d", obj->cause );
        mapPtItem_ColCause[obj->address]->setText( buf );
        sprintf( buf, "%d", 1+mapPtItem_ColCount[obj->address]->text().toInt() );
        mapPtItem_ColCount[obj->address]->setText( buf );

        switch (obj->type)
          {
          case iec104_class::M_SP_TB_1: // 1
          case iec104_class::M_SP_NA_1: // 30
              sprintf( buf, "%s%s%s%s%s", obj->scs?"on ":"off ", obj->iv?"iv ":"", obj->bl?"bl ":"", obj->sb?"sb ":"", obj->nt?"nt ":"" );
              break;
          case iec104_class::M_DP_NA_1: // 3
          case iec104_class::M_DP_TB_1: // 31
              sprintf( buf, "%s%s%s%s%s", dblmsg[obj->dcs], obj->iv?"iv ":"", obj->bl?"bl ":"", obj->sb?"sb ":"", obj->nt?"nt ":"" );
              break;
          case iec104_class::M_ST_NA_1: // 5
              sprintf( buf, "%s%s%s%s%s%s", obj->ov?"ov ":"", obj->iv?"iv ":"", obj->bl?"bl ":"", obj->sb?"sb ":"", obj->nt?"nt ":"", obj->t?"t ":"" );
              break;
          case iec104_class::M_ME_NA_1: // 9
          case iec104_class::M_ME_NB_1: // 11
          case iec104_class::M_ME_NC_1: // 13
              sprintf( buf, "%s%s%s%s%s", obj->ov?"ov ":"", obj->iv?"iv ":"", obj->bl?"bl ":"", obj->sb?"sb ":"", obj->nt?"nt ":"" );
              break;
          }

        mapPtItem_ColFlags[obj->address]->setText( buf );
    }

if ( inserted )
    ui->twPontos->sortItems ( 0 );
}

void MainWindow::slot_timer_logmsg()
{
    static int count = 0;
    static int rowant = 0;

    if ( Hide )
      if ( this->isVisible() )
         this->setVisible( false );

    // adjust size of rows and columns
    if ( ! ( ++count%15 ) )
        if ( rowant < ui->twPontos->rowCount() )
        {
        rowant = ui->twPontos->rowCount();
        ui->twPontos->resizeRowsToContents();
        ui->twPontos->resizeColumnsToContents();
        }

    // if ( !i104.mLog.haveMsg() && i104.tmKeepAlive->isActive() )
    //  i104.mLog.pushMsg( "." );

    if ( i104.mLog.haveMsg() )
    {
      if (ui->lwLog->count() > 5000)
      {
          ui->lwLog->clear();
          ui->lwLog->addItem( "*** Message list auto cleaned!" );
      }

      while ( i104.mLog.haveMsg() )
      {
          ui->lwLog->addItem( i104.mLog.pullMsg().c_str() );
      }
      if (ui->cbAutoScroll->isChecked())
        ui->lwLog->scrollToBottom();
    }
}

void  MainWindow::slot_interrogationActConfIndication()
{
msg_req m;
m.COD = T_INICIO;
m.TIPO = REQ_GRUPO;
m.ORIG = BDTR_orig;
m.ID = 0;
m.NPTS = 0;
m.PONTOS[0] = 0;
udps->writeDatagram ( (const char *) &m, sizeof( msg_req ), BDTR_host, BDTR_porta );
if ( BDTR_HaveDualHost() )
  udps->writeDatagram ( (const char *) &m, sizeof( msg_req ), BDTR_host_dual, BDTR_porta );
BDTR_Loga( "<-- BDTR: INTERROGATION BEGIN" );
}

void  MainWindow::slot_interrogationActTermIndication()
{
msg_req m;
m.COD = T_FIM;
m.TIPO = REQ_GRUPO;
m.ORIG = BDTR_orig;
m.ID = 0;
m.NPTS = 0;
m.PONTOS[0] = 0;
udps->writeDatagram ( (const char *) &m, sizeof( msg_req ), BDTR_host, BDTR_porta );
if ( BDTR_HaveDualHost() )
    udps->writeDatagram ( (const char *) &m, sizeof( msg_req ), BDTR_host_dual, BDTR_porta );
BDTR_Loga( "<-- BDTR: INTERROGATION END" );
}

void MainWindow::slot_tcpconnect()
{
    ui->lbStatus->setText( "<font color='green'> TCP CONNECTED!</font>" );
    ui->pbGI->setEnabled( true );
    ui->pbSendCommandsButton->setEnabled( true );
    ui->pbConnect->setText( "Disconnect" );
}

void MainWindow::slot_tcpdisconnect()
{
    if ( BDTR_HaveDualHost() && isPrimary == true )
    {
       isPrimary = false;
       BDTR_Loga( "--- BDTR: BECOMING SECONDARY BY DISCONNECTION" );
       ui->lbMode->setText( "<font color='red'>Secondary</font>" );
    }

    ui->lbStatus->setText( "<font color='red'> TCP DISCONNECTED!</font>" );
    ui->pbGI->setEnabled( false );
    ui->pbSendCommandsButton->setEnabled( false );

    if ( i104.tmKeepAlive->isActive() )
    {
        ui->pbConnect->setText( "Give up" );
        ui->leIPRemoto->setEnabled( false );
        ui->leLinkAddress->setEnabled( false );
        ui->leMasterAddress->setEnabled( false );
    }
    else
    {
        ui->pbConnect->setText( "Connect" );
        ui->leIPRemoto->setEnabled( true );
        ui->leLinkAddress->setEnabled( true );
        ui->leMasterAddress->setEnabled( true );
    }
}

void MainWindow::slot_commandActConfIndication( iec_obj *obj )
{
bool is_select = false;

    if ( LastCommandAddress == obj->address )
    {
        i104.mLog.pushMsg("    COMMAND ACT CONF INDICATION");
        is_select = ( obj->se == iec104_class::SELECT );

        // if confirmed select, execute
        if ( obj->se == iec104_class::SELECT && obj->pn == iec104_class::POSITIVE )
        {
            obj->se = iec104_class::EXECUTE;
            i104.sendCommand( obj );
        }

        // respond to BDTR only if it's not a select or if its a negative response
        if ( is_select == false || obj->pn == iec104_class::NEGATIVE )
        {
            char bufOut[1600];  // buffer for bdtr response
            msg_ack *ms;
            ms=(msg_ack*)bufOut;
            // ack msg for BDTR
            ms->COD = T_ACK;
            ms->TIPO = T_COM;
            ms->ORIG = BDTR_orig;
            ms->COMP = obj->address;
            switch ( obj->type )
            {
            case iec104_class::C_SC_NA_1:
            case iec104_class::C_SC_TA_1:
                ms->ID = ( obj->scs == 1 ) ? 2 : 1;
                break;
            case iec104_class::C_DC_NA_1:
            case iec104_class::C_DC_TA_1:
                ms->ID = obj->dcs;
                break;
            case iec104_class::C_RC_NA_1:
            case iec104_class::C_RC_TA_1:
                ms->ID = obj->rcs;
                break;
            }
            if ( obj->pn == iec104_class::NEGATIVE )
            {
                ms->ID |= 0x80;
                BDTR_Loga( "<-- BDTR: COMMAND REJECTED BY IEC104 SLAVE" );
            }
            else
            {
                BDTR_Loga( "<-- BDTR: COMMAND ACCEPTED BY IEC104 SLAVE" );
            }
            udps->writeDatagram ( (const char *) bufOut, sizeof(msg_ack), BDTR_host, BDTR_porta );
            if ( BDTR_HaveDualHost() )
                udps->writeDatagram ( (const char *) bufOut, sizeof(msg_ack), BDTR_host_dual, BDTR_porta );
        }
    }
};

void MainWindow::slot_commandActTermIndication( iec_obj *obj )
{
    if ( LastCommandAddress == obj->address )
      i104.mLog.pushMsg("    COMMAND ACT TERM INDICATION");
};

void MainWindow::closeEvent( QCloseEvent *event )
{
    i104.terminate();
    event->accept();
}

void MainWindow::slot_timer_BDTR_kamsg()
{
    if ( ! isPrimary )
    {
       if ( BDTR_CntDnToBePrimary <= 0 )
          {
          isPrimary = true;
          i104.enable_connect();
          BDTR_CntDnToBePrimary = BDTR_CntToBePrimary;
          BDTR_Loga( "--- BDTR: BECOMING PRIMARY BY TIMEOUT" );
          ui->lbMode->setText( "<font color='green'>Primary</font>" );
          }
       else
          BDTR_CntDnToBePrimary--;
    }

    if ( isPrimary )
    {   // send keepalive message to the dual host

        msg_sinc m;
        m.COD = T_HORA;
        m.VAGO = 0;
        m.ORIG = BDTR_orig;

        time_t timer;
        struct tm *tblock;
        timer = time(NULL); // gets time of day
        tblock = localtime(&timer); // converts date/time to a structure

        m.TAG.ANO = tblock->tm_year+1900;
        m.TAG.MES = tblock->tm_mon+1;
        m.TAG.DIA = tblock->tm_mday;
        m.TAG.HORA = tblock->tm_hour;
        m.TAG.MINUTO = tblock->tm_min;
        m.TAG.MSEGS = 1000*tblock->tm_sec+0;

        udps->writeDatagram ( (const char *) &m, sizeof(m), BDTR_host_dual, BDTR_porta_escuta );
    }
}

void MainWindow::on_cbLog_clicked()
{
    if ( ui->cbLog->isChecked() )
        i104.mLog.activateLog();
    else
        i104.mLog.deactivateLog();
}
