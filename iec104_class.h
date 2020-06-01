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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef IEC104_CLASS_H
#define IEC104_CLASS_H

// IEC 60870-5-104 BASE CLASS, MASTER IMPLEMENTATION

#include "iec104_types.h"
#include "logmsg.h"

struct iec_obj {
    unsigned int address;       // 3 byte address           3字节地址

    float value;                // value                    值

    cp56time2a timetag;         // 7 byte time tag          7字节时间标签
    unsigned char reserved;     // for future use           保留

    unsigned char type;         // iec type                 IEC类型
    unsigned char cause;        //
    unsigned short ca;          // common addres of asdu    ASDU地址

    union {
        unsigned char ov :1;    // overflow/no overflow     溢出/不溢出
        unsigned char sp :1;    // single point information 单点信息
        unsigned char dp :2;    // double point information 双点信息
        unsigned char scs :1;   // single command state     单个命令状态
        unsigned char dcs :2;   // double command state     双指令状态
        unsigned char rcs :2;   // regulating step command  调节步进命令
    }; // 2 bits

    unsigned char qu :5;        // qualifier of command  命令限定符
    unsigned char se :1;        // select=1 / execute=0  选择= 1 /执行= 0
    // + 6 bits = 8 bits

    unsigned char bl :1;        // blocked/not blocked         阻止/未阻止
    unsigned char sb :1;        // substituted/not substituted 取代/不取代
    unsigned char nt :1;        // not topical/topical         不是主题/主题
    unsigned char iv :1;        // valid/invalid               有效/无效
    unsigned char t :1;         // transient flag              瞬态标志
    unsigned char pn :1;        // 0=positive, 1=negative      0 =正，1 =负
};

class iec104_class
{
    public:

    static const unsigned int M_SP_NA_1 = 1;    // single-point information                单点信息
    static const unsigned int M_DP_NA_1 = 3;    // double-point information                双点信息
    static const unsigned int M_ST_NA_1 = 5;    // step position information               步骤位置信息
    static const unsigned int M_BO_NA_1 = 7;    // bitstring of 32 bits                    32位的位串
    static const unsigned int M_ME_NA_1 = 9;    // normalized value                        归一化值
    static const unsigned int M_ME_NB_1 = 11;   // scaled value                            标度值
    static const unsigned int M_ME_NC_1 = 13;   // floating point                          浮点
    static const unsigned int M_IT_NA_1 = 15;   // integrated totals                       综合总计
    static const unsigned int M_SP_TB_1 = 30;   // single-point information with time tag  带时间标签的单点信息
    static const unsigned int M_DP_TB_1 = 31;   // double-point information with time tag  带时间标签的双点信息
    static const unsigned int M_ST_TB_1 = 32;   // step position information with time tag 带时间标签的步位置信息
    static const unsigned int M_BO_TB_1 = 33;   // bitstring of 32 bits with time tag      带时间标签的32位的位串
    static const unsigned int M_ME_TD_1 = 34;   // normalized value with time tag          带时间标签的归一化值
    static const unsigned int M_ME_TE_1 = 35;   // scaled value with time tag              带时间标签的标定值
    static const unsigned int M_ME_TF_1 = 36;   // floating point with time tag            带时间标记的浮点
    static const unsigned int M_IT_TB_1 = 37;   // integrated totals with time tag         带时间标签的累计总数
    static const unsigned int C_SC_NA_1 = 45;   // single command                          单指令
    static const unsigned int C_DC_NA_1 = 46;   // double command                          双指令
    static const unsigned int C_RC_NA_1 = 47;   // regulating step command                 调节阶跃命令
    static const unsigned int C_SC_TA_1 = 58;   // single command with time tag            带时间标签的单个命令
    static const unsigned int C_DC_TA_1 = 59;   // double command with time tag            带时间标签的双重命令
    static const unsigned int C_RC_TA_1 = 60;   // regulating step command with time tag   带时间标签的调节步进命令
    static const unsigned int M_EI_NA_1 = 70;   // end of initialization                   初始化结束
    static const unsigned int C_IC_NA_1 = 100;  // general interrogation (GI)              一般审讯（GI）
    static const unsigned int C_CI_NA_1 = 101;  // counter interrogation                   反审讯
    static const unsigned int C_CS_NA_1 = 103;  // clock synchronization command           时钟同步命令
    static const unsigned int C_RP_NA_1 = 105;  // reset process command                   重置过程命令
    static const unsigned int C_TS_TA_1 = 107;  // test command with time tag CP56Time2a   带时间标签CP56Time2a的测试命令

    /* cause of transmition (standard) */
    /* 传播原因（标准） */
    static const unsigned int CYCLIC = 1;
    static const unsigned int BGSCAN = 2;
    static const unsigned int SPONTANEOUS = 3;
    static const unsigned int REQUEST = 5;
    static const unsigned int ACTIVATION = 6;
    static const unsigned int ACTCONFIRM = 7;
    static const unsigned int DEACTIVATION = 8;
    static const unsigned int ACTTERM = 10;

    static const unsigned int SUPERVISORY = 0x01;
    static const unsigned int STARTDTACT = 0x07;
    static const unsigned int STARTDTCON = 0x0B;
    static const unsigned int STOPDTACT = 0x13;
    static const unsigned int STOPDTCON = 0x23;
    static const unsigned int TESTFRACT = 0x43;
    static const unsigned int TESTFRCON = 0x83;
    static const unsigned int INTERROGATION = 0x64;
    static const unsigned int START = 0x68;
    static const unsigned int RESET = 0x69;

    static const unsigned int POSITIVE = 0;
    static const unsigned int NEGATIVE = 1;

    static const unsigned int SELECT = 1;
    static const unsigned int EXECUTE = 0;

    TLogMsg mLog;

    // ---- user called funcions, must be called by the user -----------------
    // ---- 用户称为funcions，必须由用户调用 -----------------
    iec104_class(); // user called constructor on derived class                             用户在派生类上调用了构造函数
    void onConnectTCP(); // user called, when tcp connected                                 当tcp连接时用户调用
    void onDisconnectTCP(); // user called, when tcp disconnected                           tcp断开连接时，用户调用
    void onTimerSecond();  // user called, each second timer                                用户呼叫，每秒钟计时器
    void packetReadyTCP(); // user called, when packet ready to be read from tcp connection 当数据包准备从tcp连接读取时，用户调用

    void solicitGI();  // General Interrogation     一般审讯
    void setSecondaryIP( char * ip );
    char * getSecondaryIP();
    void setSecondaryAddress( int addr );
    int getSecondaryAddress();
    void setPrimaryAddress( int addr );
    int getPrimaryAddress();
    void disableSequenceOrderCheck();  // allow sequence out of order           允许顺序混乱
    bool sendCommand( iec_obj *obj ); // Command, return false if not send      命令，如果不发送，则返回false
    int getPortTCP();
    void setPortTCP( unsigned port );

    private:
    unsigned short VS;  // sender packet control counter                    发件人数据包控制计数器
    unsigned short VR;  // receiver packet control counter                  接收者数据包控制计数器
    void confTestCommand(); // test command activation confirmation         测试命令激活确认
    void sendStartDTACT(); // send STARTDTACT                               发送STARTDTACT
    int tout_startdtact; // timeout control                                 超时控制
    void sendSupervisory(); // send supervisory window control frame        发送监控窗口控制框
    int tout_supervisory;  // countdown to send supervisory window control  倒计时发送监控窗口控件
    int tout_gi; // countdown to send general interrogation                 倒计时发送一般审讯
    int tout_testfr; // countdown to send test frame                        倒数发送测试帧
    bool connectedTCP; // tcp connection state                              TCP连接状态
    bool seq_order_check; // if set: test message order, disconnect if out of order                     如果设置：测试消息顺序，如果故障则断开连接
    unsigned char masterAddress; // master link address (primary address, originator address, oa)       主链接地址（主地址，发起方地址，oa）
    unsigned short slaveAddress; // slave link address (secondary address, common address of ASDU, ca)  从站链接地址（辅助地址，ASDU的公共地址，ca）
    unsigned Port; // iec104 tcp port (defaults to 2404)                                                iec104 tcp端口（默认为2404）
    char slaveIP[20]; // slave (secondary, RTU) IP address                                              从属（辅助，RTU）IP地址
    static const int t3_testfr = 10;
    static const int t2_supervisory = 8;
    static const int t1_startdtact = 6;

    protected:
    void parseAPDU(iec_apdu * papdu, int sz, bool accountandrespond = true); // parse APDU, ( accountandrespond == false : process the apdu out of the normal handshake )

    int msg_supervisory;

    bool TxOk; // ready to transmit state (STARTDTCON received)             准备发送状态（已收到STARTDTCON）
    unsigned GIObjectCnt; // contador de objetos da GI                      GI对象计数器

    // ---- pure virtual funcions, user defined on derived class (mandatory)--- 纯虚函数，用户在派生类中定义（强制性）

    // make tcp connection, user provided
    // 建立tcp连接，用户提供
    virtual void connectTCP() = 0;
    // tcp disconnect, user provided
    // tcp断开连接，用户提供
    virtual void disconnectTCP() = 0;
    // read tcp data, user provided
    // 读取tcp数据，用户提供
    virtual int readTCP( char * buf, int szmax ) = 0;
    // send tcp data, user provided
    // 发送tcp数据，用户提供
    virtual void sendTCP( char * data, int sz ) = 0;

    // ---- virtual funcions, user defined on derived class (not mandatory)---

    // user point process, user provided. (on one call must be only objects of one type)
    // 用户点过程，由用户提供。 （一次调用只能是一种类型的对象）
    virtual void dataIndication( iec_obj * /*obj*/, int /*numpoints*/){};
    // inform user that ACTCONFIRM of Interrogation was received from slave
    // 通知用户从接收到了ACTCONFIRM的询问
    virtual void interrogationActConfIndication(){};
    // inform user that ACTTERM of Interrogation was received from slave
    // 通知用户从接收到了ACTTERM的询问
    virtual void interrogationActTermIndication(){};
    // inform user of command activation
    // 通知用户命令激活
    virtual void commandActConfIndication( iec_obj * /*obj*/ ){};
    // inform user of command termination
    // 通知用户命令终止
    virtual void commandActTermIndication( iec_obj * /*obj*/ ){};
    // user process APDU
    // 用户进程APDU
    virtual void userprocAPDU(iec_apdu * /* papdu */, int /* sz */){};

    // -------------------------------------------------------------------------

};

#endif // IEC104_CLASS_H


