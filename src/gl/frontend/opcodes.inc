/* ANSI-C code produced by gperf version 3.0.1 */
/* Command-line: gperf -t -c -Ktext --null-strings --hash-function=OpcodeHash --lookup-function=CheckOpcode -G -CP --language=ANSI-C -multiple-iterations=20 opcodes.txt  */
/* Computed positions: -k'1-3,8,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "opcodes.txt"
struct Keyword {
	char *		text;
	Opcode		opcode;
};

#define TOTAL_KEYWORDS 114
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 11
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 343
/* maximum key range = 341, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
OpcodeHash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      125,  35, 120, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344,  10,   2,  20,  35,  15,
      100,  65,  12,   5, 344,   0,  25,  75,  55,  90,
        5, 110,   5,  35,   0,  70,  20,  35,   0, 344,
       85, 344, 344, 344, 344, 344, 344,   0,  10,  10,
        0,   0,   0,   0,   0,   0, 344, 344,   0,   0,
        5,   0, 344, 344, 344,   0,   0, 344,   0,   0,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344, 344, 344, 344, 344,
      344, 344, 344, 344, 344, 344
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str3[sizeof("low")];
    char stringpool_str4[sizeof("high")];
    char stringpool_str5[sizeof("float")];
    char stringpool_str6[sizeof("medium")];
    char stringpool_str7[sizeof("TXB")];
    char stringpool_str8[sizeof("int")];
    char stringpool_str9[sizeof("TXB_SAT")];
    char stringpool_str10[sizeof("BRK")];
    char stringpool_str12[sizeof("TXP_SAT")];
    char stringpool_str13[sizeof("TXP")];
    char stringpool_str14[sizeof("bool")];
    char stringpool_str18[sizeof("TEX")];
    char stringpool_str22[sizeof("TEX_SAT")];
    char stringpool_str23[sizeof("RET")];
    char stringpool_str27[sizeof("EXP_SAT")];
    char stringpool_str28[sizeof("EXP")];
    char stringpool_str30[sizeof("PHI")];
    char stringpool_str31[sizeof("samplerCube")];
    char stringpool_str32[sizeof("TXL_SAT")];
    char stringpool_str33[sizeof("REP")];
    char stringpool_str37[sizeof("RCP_SAT")];
    char stringpool_str38[sizeof("RCP")];
    char stringpool_str39[sizeof("mat3")];
    char stringpool_str40[sizeof("ivec3")];
    char stringpool_str42[sizeof("LRP_SAT")];
    char stringpool_str43[sizeof("LRP")];
    char stringpool_str47[sizeof("XPD_SAT")];
    char stringpool_str48[sizeof("STR")];
    char stringpool_str49[sizeof("vec3")];
    char stringpool_str50[sizeof("bvec3")];
    char stringpool_str53[sizeof("TXL")];
    char stringpool_str54[sizeof("ABS_SAT")];
    char stringpool_str58[sizeof("KIL")];
    char stringpool_str59[sizeof("DPH_SAT")];
    char stringpool_str63[sizeof("SLT")];
    char stringpool_str67[sizeof("DPH")];
    char stringpool_str68[sizeof("ARL")];
    char stringpool_str70[sizeof("INPUT")];
    char stringpool_str73[sizeof("DST")];
    char stringpool_str77[sizeof("DST_SAT")];
    char stringpool_str78[sizeof("XPD")];
    char stringpool_str79[sizeof("sampler3D")];
    char stringpool_str82[sizeof("DP3_SAT")];
    char stringpool_str83[sizeof("CAL")];
    char stringpool_str85[sizeof("ABS")];
    char stringpool_str87[sizeof("ADD_SAT")];
    char stringpool_str88[sizeof("MAX")];
    char stringpool_str92[sizeof("MAX_SAT")];
    char stringpool_str93[sizeof("SLE")];
    char stringpool_str94[sizeof("ELSE")];
    char stringpool_str97[sizeof("SCS_SAT")];
    char stringpool_str98[sizeof("SCC")];
    char stringpool_str99[sizeof("TEMP")];
    char stringpool_str100[sizeof("PARAM")];
    char stringpool_str102[sizeof("SIN_SAT")];
    char stringpool_str103[sizeof("SGT")];
    char stringpool_str107[sizeof("CMP_SAT")];
    char stringpool_str108[sizeof("CMP")];
    char stringpool_str112[sizeof("SUB")];
    char stringpool_str113[sizeof("DP3")];
    char stringpool_str114[sizeof("SUB_SAT")];
    char stringpool_str116[sizeof("ENDREP")];
    char stringpool_str117[sizeof("ENDLOOP")];
    char stringpool_str118[sizeof("ADD")];
    char stringpool_str122[sizeof("ADDRESS")];
    char stringpool_str123[sizeof("SNE")];
    char stringpool_str124[sizeof("mat4")];
    char stringpool_str125[sizeof("ivec4")];
    char stringpool_str127[sizeof("MAD_SAT")];
    char stringpool_str128[sizeof("SCS")];
    char stringpool_str129[sizeof("mat2")];
    char stringpool_str130[sizeof("ivec2")];
    char stringpool_str132[sizeof("FRC_SAT")];
    char stringpool_str133[sizeof("SGE")];
    char stringpool_str134[sizeof("vec4")];
    char stringpool_str135[sizeof("bvec4")];
    char stringpool_str137[sizeof("FLR_SAT")];
    char stringpool_str138[sizeof("FLR")];
    char stringpool_str139[sizeof("vec2")];
    char stringpool_str140[sizeof("bvec2")];
    char stringpool_str142[sizeof("MIN_SAT")];
    char stringpool_str147[sizeof("EX2_SAT")];
    char stringpool_str148[sizeof("FRC")];
    char stringpool_str152[sizeof("COS_SAT")];
    char stringpool_str153[sizeof("SIN")];
    char stringpool_str157[sizeof("RSQ_SAT")];
    char stringpool_str158[sizeof("MAD")];
    char stringpool_str162[sizeof("SWZ_SAT")];
    char stringpool_str166[sizeof("OUTPUT")];
    char stringpool_str167[sizeof("DP4_SAT")];
    char stringpool_str168[sizeof("POW")];
    char stringpool_str169[sizeof("sampler2D")];
    char stringpool_str172[sizeof("DP2_SAT")];
    char stringpool_str177[sizeof("MUL_SAT")];
    char stringpool_str183[sizeof("COS")];
    char stringpool_str187[sizeof("LOG_SAT")];
    char stringpool_str188[sizeof("SFL")];
    char stringpool_str192[sizeof("MOV_SAT")];
    char stringpool_str193[sizeof("MIN")];
    char stringpool_str198[sizeof("MUL")];
    char stringpool_str203[sizeof("SSG")];
    char stringpool_str207[sizeof("IF")];
    char stringpool_str208[sizeof("MOV")];
    char stringpool_str210[sizeof("ENDIF")];
    char stringpool_str214[sizeof("LOOP")];
    char stringpool_str222[sizeof("LG2_SAT")];
    char stringpool_str243[sizeof("SWZ")];
    char stringpool_str248[sizeof("LOG")];
    char stringpool_str263[sizeof("RSQ")];
    char stringpool_str268[sizeof("EX2")];
    char stringpool_str273[sizeof("SEQ")];
    char stringpool_str283[sizeof("DP4")];
    char stringpool_str293[sizeof("DP2")];
    char stringpool_str343[sizeof("LG2")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "low",
    "high",
    "float",
    "medium",
    "TXB",
    "int",
    "TXB_SAT",
    "BRK",
    "TXP_SAT",
    "TXP",
    "bool",
    "TEX",
    "TEX_SAT",
    "RET",
    "EXP_SAT",
    "EXP",
    "PHI",
    "samplerCube",
    "TXL_SAT",
    "REP",
    "RCP_SAT",
    "RCP",
    "mat3",
    "ivec3",
    "LRP_SAT",
    "LRP",
    "XPD_SAT",
    "STR",
    "vec3",
    "bvec3",
    "TXL",
    "ABS_SAT",
    "KIL",
    "DPH_SAT",
    "SLT",
    "DPH",
    "ARL",
    "INPUT",
    "DST",
    "DST_SAT",
    "XPD",
    "sampler3D",
    "DP3_SAT",
    "CAL",
    "ABS",
    "ADD_SAT",
    "MAX",
    "MAX_SAT",
    "SLE",
    "ELSE",
    "SCS_SAT",
    "SCC",
    "TEMP",
    "PARAM",
    "SIN_SAT",
    "SGT",
    "CMP_SAT",
    "CMP",
    "SUB",
    "DP3",
    "SUB_SAT",
    "ENDREP",
    "ENDLOOP",
    "ADD",
    "ADDRESS",
    "SNE",
    "mat4",
    "ivec4",
    "MAD_SAT",
    "SCS",
    "mat2",
    "ivec2",
    "FRC_SAT",
    "SGE",
    "vec4",
    "bvec4",
    "FLR_SAT",
    "FLR",
    "vec2",
    "bvec2",
    "MIN_SAT",
    "EX2_SAT",
    "FRC",
    "COS_SAT",
    "SIN",
    "RSQ_SAT",
    "MAD",
    "SWZ_SAT",
    "OUTPUT",
    "DP4_SAT",
    "POW",
    "sampler2D",
    "DP2_SAT",
    "MUL_SAT",
    "COS",
    "LOG_SAT",
    "SFL",
    "MOV_SAT",
    "MIN",
    "MUL",
    "SSG",
    "IF",
    "MOV",
    "ENDIF",
    "LOOP",
    "LG2_SAT",
    "SWZ",
    "LOG",
    "RSQ",
    "EX2",
    "SEQ",
    "DP4",
    "DP2",
    "LG2"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct Keyword wordlist[] =
  {
    {-1}, {-1}, {-1},
#line 99 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str3,OpcodeLow},
#line 101 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str4,OpcodeMedium},
#line 110 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str5,OpcodeFloat},
#line 100 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str6,OpcodeMedium},
#line 75 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str7,OpcodeTXB},
#line 106 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str8,OpcodeInt},
#line 76 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str9,OpcodeTXB_SAT},
#line 83 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str10,OpcodeBRK},
    {-1},
#line 80 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str12,OpcodeTXP_SAT},
#line 79 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str13,OpcodeTXP},
#line 102 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str14,OpcodeBool},
    {-1}, {-1}, {-1},
#line 73 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str18,OpcodeTEX},
    {-1}, {-1}, {-1},
#line 74 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str22,OpcodeTEX_SAT},
#line 53 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str23,OpcodeRET},
    {-1}, {-1}, {-1},
#line 29 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str27,OpcodeEXP_SAT},
#line 28 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str28,OpcodeEXP},
    {-1},
#line 93 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str30,OpcodePHI},
#line 119 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str31,OpcodeSamplerCube},
#line 78 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str32,OpcodeTXL_SAT},
#line 90 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str33,OpcodeREP},
    {-1}, {-1}, {-1},
#line 52 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str37,OpcodeRCP_SAT},
#line 51 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str38,OpcodeRCP},
#line 115 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str39,OpcodeFloatMat3},
#line 108 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str40,OpcodeIntVec3},
    {-1},
#line 39 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str42,OpcodeLRP_SAT},
#line 38 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str43,OpcodeLRP},
    {-1}, {-1}, {-1},
#line 82 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str47,OpcodeXPD_SAT},
#line 68 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str48,OpcodeSTR},
#line 112 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str49,OpcodeFloatVec3},
#line 104 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str50,OpcodeBoolVec3},
    {-1}, {-1},
#line 77 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str53,OpcodeTXL},
#line 8 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str54,OpcodeABS_SAT},
    {-1}, {-1}, {-1},
#line 91 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str58,OpcodeKIL},
#line 23 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str59,OpcodeDPH_SAT},
    {-1}, {-1}, {-1},
#line 65 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str63,OpcodeSLT},
    {-1}, {-1}, {-1},
#line 22 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str67,OpcodeDPH},
#line 6 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str68,OpcodeARL},
    {-1},
#line 94 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str70,OpcodeINPUT},
    {-1}, {-1},
#line 24 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str73,OpcodeDST},
    {-1}, {-1}, {-1},
#line 25 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str77,OpcodeDST_SAT},
#line 81 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str78,OpcodeXPD},
#line 118 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str79,OpcodeSampler3D},
    {-1}, {-1},
#line 19 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str82,OpcodeDP3_SAT},
#line 11 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str83,OpcodeCAL},
    {-1},
#line 7 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str85,OpcodeABS},
    {-1},
#line 10 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str87,OpcodeADD_SAT},
#line 42 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str88,OpcodeMAX},
    {-1}, {-1}, {-1},
#line 43 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str92,OpcodeMAX_SAT},
#line 64 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str93,OpcodeSLE},
#line 84 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str94,OpcodeELSE},
    {-1}, {-1},
#line 57 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str97,OpcodeSCS_SAT},
#line 92 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str98,OpcodeSCC},
#line 96 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str99,OpcodeTEMP},
#line 97 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str100,OpcodePARAM},
    {-1},
#line 63 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str102,OpcodeSIN_SAT},
#line 61 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str103,OpcodeSGT},
    {-1}, {-1}, {-1},
#line 13 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str107,OpcodeCMP_SAT},
#line 12 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str108,OpcodeCMP},
    {-1}, {-1}, {-1},
#line 69 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str112,OpcodeSUB},
#line 18 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str113,OpcodeDP3},
#line 70 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str114,OpcodeSUB_SAT},
    {-1},
#line 87 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str116,OpcodeENDREP},
#line 86 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str117,OpcodeENDLOOP},
#line 9 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str118,OpcodeADD},
    {-1}, {-1}, {-1},
#line 98 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str122,OpcodeADDRESS},
#line 66 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str123,OpcodeSNE},
#line 116 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str124,OpcodeFloatMat4},
#line 109 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str125,OpcodeIntVec4},
    {-1},
#line 41 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str127,OpcodeMAD_SAT},
#line 56 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str128,OpcodeSCS},
#line 114 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str129,OpcodeFloatMat2},
#line 107 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str130,OpcodeIntVec2},
    {-1},
#line 33 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str132,OpcodeFRC_SAT},
#line 60 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str133,OpcodeSGE},
#line 113 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str134,OpcodeFloatVec4},
#line 105 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str135,OpcodeBoolVec4},
    {-1},
#line 31 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str137,OpcodeFLR_SAT},
#line 30 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str138,OpcodeFLR},
#line 111 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str139,OpcodeFloatVec2},
#line 103 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str140,OpcodeBoolVec2},
    {-1},
#line 45 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str142,OpcodeMIN_SAT},
    {-1}, {-1}, {-1}, {-1},
#line 27 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str147,OpcodeEX2_SAT},
#line 32 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str148,OpcodeFRC},
    {-1}, {-1}, {-1},
#line 15 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str152,OpcodeCOS_SAT},
#line 62 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str153,OpcodeSIN},
    {-1}, {-1}, {-1},
#line 55 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str157,OpcodeRSQ_SAT},
#line 40 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str158,OpcodeMAD},
    {-1}, {-1}, {-1},
#line 72 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str162,OpcodeSWZ_SAT},
    {-1}, {-1}, {-1},
#line 95 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str166,OpcodeOUTPUT},
#line 21 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str167,OpcodeDP4_SAT},
#line 50 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str168,OpcodePOW},
#line 117 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str169,OpcodeSampler2D},
    {-1}, {-1},
#line 17 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str172,OpcodeDP2_SAT},
    {-1}, {-1}, {-1}, {-1},
#line 49 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str177,OpcodeMUL_SAT},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 14 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str183,OpcodeCOS},
    {-1}, {-1}, {-1},
#line 37 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str187,OpcodeLOG_SAT},
#line 59 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str188,OpcodeSFL},
    {-1}, {-1}, {-1},
#line 47 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str192,OpcodeMOV_SAT},
#line 44 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str193,OpcodeMIN},
    {-1}, {-1}, {-1}, {-1},
#line 48 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str198,OpcodeMUL},
    {-1}, {-1}, {-1}, {-1},
#line 67 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str203,OpcodeSSG},
    {-1}, {-1}, {-1},
#line 88 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str207,OpcodeIF},
#line 46 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str208,OpcodeMOV},
    {-1},
#line 85 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str210,OpcodeENDIF},
    {-1}, {-1}, {-1},
#line 89 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str214,OpcodeLOOP},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 35 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str222,OpcodeLG2_SAT},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 71 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str243,OpcodeSWZ},
    {-1}, {-1}, {-1}, {-1},
#line 36 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str248,OpcodeLOG},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 54 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str263,OpcodeRSQ},
    {-1}, {-1}, {-1}, {-1},
#line 26 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str268,OpcodeEX2},
    {-1}, {-1}, {-1}, {-1},
#line 58 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str273,OpcodeSEQ},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 20 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str283,OpcodeDP4},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 16 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str293,OpcodeDP2},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1},
#line 34 "opcodes.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str343,OpcodeLG2}
  };

#ifdef __GNUC__
__inline
#endif
const struct Keyword *
CheckOpcode (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = OpcodeHash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int o = wordlist[key].text;
          if (o >= 0)
            {
              register const char *s = o + stringpool;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist[key];
            }
        }
    }
  return 0;
}
