/*
 * Copyright (C) 2005 by Grigoriy A. Sitkarev
 * sitkarev@komi.tgk-9.ru
 *
 * Adapted by Ricardo Olsen from original in http://mrts-ng.googlecode.com
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

#ifndef __IEC104_TYPES_H
#define __IEC104_TYPES_H

#pragma pack(push,1)

// 32-bit string state and change data unit
// 32位字符串状态和更改数据单元
struct iec_stcd {
    union {
        unsigned short  st;
        struct {
            unsigned char st1 :1;
            unsigned char st2 :1;
            unsigned char st3 :1;
            unsigned char st4 :1;
            unsigned char st5 :1;
            unsigned char st6 :1;
            unsigned char st7 :1;
            unsigned char st8 :1;
            unsigned char st9 :1;
            unsigned char st10 :1;
            unsigned char st11 :1;
            unsigned char st12 :1;
            unsigned char st13 :1;
            unsigned char st14 :1;
            unsigned char st15 :1;
            unsigned char st16 :1;
        };
    };
    union {
        unsigned short  cd;
        struct {
            unsigned char cd1 :1;
            unsigned char cd2 :1;
            unsigned char cd3 :1;
            unsigned char cd4 :1;
            unsigned char cd5 :1;
            unsigned char cd6 :1;
            unsigned char cd7 :1;
            unsigned char cd8 :1;
            unsigned char cd9 :1;
            unsigned char cd10 :1;
            unsigned char cd11 :1;
            unsigned char cd12 :1;
            unsigned char cd13 :1;
            unsigned char cd14 :1;
            unsigned char cd15 :1;
            unsigned char cd16 :1;
        };
    };
};

// CP56Time2a timestamp
// CP56Time2a时间戳
struct cp56time2a {
    unsigned short msec;
    unsigned char min :6;
    unsigned char res1 :1;
    unsigned char iv :1;
    unsigned char hour :5;
    unsigned char res2 :2;
    unsigned char su :1;
    unsigned char mday :5;
    unsigned char wday :3;
    unsigned char month :4;
    unsigned char res3 :4;
    unsigned char year :7;
    unsigned char res4 :1;
};

typedef struct cp56time2a cp56time2a;

// 限定词
struct iec_qualif
{
    unsigned char var :2;
    unsigned char res :2;
    unsigned char bl :1; // blocked/not blocked             被阻止/未被阻止
    unsigned char sb :1; // substituted/not substituted     替换/不替换
    unsigned char nt :1; // not topical/topical             不是主题/主题
    unsigned char iv :1; // valid/invalid                   有效/无效
};

// M_SP_NA_1 - single point information with quality description
// M_SP_NA_1 - 具有质量描述的单点信息
struct iec_type1 {
    unsigned char sp :1; // single point information
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_DP_NA_1 - double point information with quality description
// M_DP_NA_1 - 具有质量描述的双点信息
struct iec_type3 {
    unsigned char dp :2; // double point information
    unsigned char res :2;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_ST_NA_1 - step position
// M_ST_NA_1 - 步骤位置
struct iec_type5 {
    unsigned char mv  :7; // value
    unsigned char t :1; // transient flag
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_BO_NA_1 - state and change information bit string
// M_BO_NA_1 - 状态和更改信息位字符串
struct iec_type7 {
    struct iec_stcd stcd;
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_ME_NA_1 - normalized measured value
// M_ME_NA_1 - 标准化测量值
struct iec_type9 {
    unsigned short mv; // normalized value
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_ME_NB_1 - scaled measured value
// M_ME_NB_1 - 标定的测量值
struct iec_type11 {
    unsigned short mv; // scaled value
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_ME_NC_1 - short floating point measured value
// M_ME_NC_1 - 短浮点测量值
struct iec_type13 {
    float mv;
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
};

// M_SP_TB_1 - single point information with quality description and time tag
// M_SP_TB_1 - 具有质量描述和时间标记的单点信息
struct iec_type30 {
    unsigned char sp :1; // single point information
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_DP_TB_1 - double point information with quality description and time tag
// M_DP_TB_1 - 具有质量描述和时间标签的双点信息
struct iec_type31 {
    unsigned char dp :2; // double point information
    unsigned char res :2;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_ST_TB_1 - step position with time tag
// M_ST_TB_1 - 带时间标记的步位置
struct iec_type32 {
    unsigned char mv  :7; // value
    unsigned char t :1; // transient flag
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_BO_TB_1 - state and change information bit string and time tag
// M_BO_TB_1 - 状态和更改信息位字符串和时间标记
struct iec_type33 {
    struct iec_stcd stcd;
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_ME_TD_1 - scaled measured value with time tag
// M_ME_TD_1 - 带时间标签的比例测量值
struct iec_type34 {
    unsigned short mv; // scaled value
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_ME_TE_1 - scaled measured value with time tag
// M_ME_TE_1 - 带有时间标签的比例测量值
struct iec_type35 {
    unsigned short mv; // scaled value
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_ME_TF_1 - short floating point measurement value and time tag
// M_ME_TF_1 - 时间标记的短浮点测量值
struct iec_type36 {
    float  mv;
    unsigned char ov :1; // overflow/no overflow
    unsigned char res :3;
    unsigned char bl :1; // blocked/not blocked
    unsigned char sb :1; // substituted/not substituted
    unsigned char nt :1; // not topical/topical
    unsigned char iv :1; // valid/invalid
    cp56time2a time;
};

// M_IT_TB_1	= 37 ,  //带 CP56Time2a 时标的累计量
struct iec_type37 {
    unsigned long int bcr;
    unsigned char sq :5;
    unsigned char cy :1;
    unsigned char ca :1;
    unsigned char iv :1;
    cp56time2a time;
};

// C_SC_NA_1	= 45	,	//	单点遥控
struct iec_type45 {
    unsigned char scs :1; // single command state
    unsigned char res :1; // must be zero
    unsigned char qu :5;
    unsigned char se :1; // select=1 / execute=0
};

// C_DC_NA_1	= 46	,	//	双点遥控
struct iec_type46 {
    unsigned char dcs :2; // double command state
    unsigned char qu :5;
    unsigned char se :1; // select=1 / execute=0
};

// C_RC_NA_1	= 47	,	//	升降命令
struct iec_type47 {
    unsigned char rcs :2; // regulating step command
    unsigned char qu :5;
    unsigned char se :1; // select=1 / execute=0
};

// C_SC_TA_1 	= 58	,	//	带长时标单点遥控(CP56)
struct iec_type58 {
    unsigned char scs :1; // single command state
    unsigned char res :1; // must be zero
    unsigned char qu :5;
    unsigned char se :1; // select=1 / execute=0
    cp56time2a time;
};

// C_DC_TA_1 	= 59	,	//	带长时标双点遥控(CP56)
struct iec_type59 {
    unsigned char dcs :2; // double command state
    unsigned char qu :5;
    unsigned char se  :1; // select=1 / execute=0
    cp56time2a time;
};

// C_RC_TA_1	= 60	,	//	带长时标升降命令(CP56)
struct iec_type60 {
    unsigned char rcs :2; // regulating step command
    unsigned char qu :5;
    unsigned char se  :1; // select=1 / execute=0
    cp56time2a time;
};

// C_IC_NA_1 	= 100	,	//	总召唤
struct iec_type100 {
    unsigned char qoi; // pointer of interrogation
};

// C_CI_NA_1 	= 101	,	//	电能脉冲召唤命令
struct iec_type101 {
    unsigned char rqt :6; // request
    unsigned char frz :2; // freeze
};

// C_CS_NA_1 	= 103	,	//	时钟同步命令
struct iec_type103 {
    cp56time2a time;
};

// C_TS_TA_1 	= 107	,	//	带长时标测试命令
struct iec_type107 {
    unsigned short ioa16; // object address bytes 1,2
    unsigned char ioa8; // object address byte 3
    unsigned short tsc; // TSC test sequence counter
    cp56time2a time;
};

// Data unit identifier block - ASDU header
// 数据单元标识符块 - ASDU标头
struct iec_unit_id {
    unsigned char type;     // type identification                  类型识别
    unsigned char num :7;   // number of information objects        信息对象的数量
    unsigned char sq :1;    // sequenced/not sequenced address      有序/无序地址
    unsigned char cause :6; // cause of transmission                传播原因
    unsigned char pn :1;    // positive/negative app. confirmation  正面/负面应用。 确认
    unsigned char t :1;     // test                                 测试
    unsigned char oa;       // originator addres                    发起者地址
    unsigned short ca;      // common address of ASDU               ASDU的公用地址
};

struct iec_apdu {
    unsigned char start;
    unsigned char length;
    unsigned short NS;
    unsigned short NR;
    struct iec_unit_id asduh;
    union {
        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type1 obj[1];
        } sq1;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type1 obj;
        } nsq1[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type3 obj[1];
        } sq3;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type3 obj;
        } nsq3[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type5 obj[1];
        } sq5;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type5 obj;
        } nsq5[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type9 obj[1];
        } sq9;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type9 obj;
        } nsq9[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type11 obj[1];
        } sq11;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type11 obj;
        } nsq11[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type13 obj[1];
        } sq13;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type13 obj;
        } nsq13[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type30 obj[1];
        } sq30;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type30 obj;
        } nsq30[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type31 obj[1];
        } sq31;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type31 obj;
        } nsq31[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type32 obj[1];
        } sq32;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type32 obj;
        } nsq32[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type34 obj[1];
        } sq34;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type34 obj;
        } nsq34[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type35 obj[1];
        } sq35;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type35 obj;
        } nsq35[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type36 obj[1];
        } sq36;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type36 obj;
        } nsq36[1];

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type45 obj;
        } nsq45;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type46 obj;
        } nsq46;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type47 obj;
        } nsq47;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type58 obj;
        } nsq58;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type59 obj;
        } nsq59;

        struct {
            unsigned short ioa16;
            unsigned char ioa8;
            iec_type60 obj;
        } nsq60;

        unsigned char dados[255];
        iec_type107 asdu107;

    };
};

#pragma pack(pop)

#endif // __IEC104_TYPES_H
