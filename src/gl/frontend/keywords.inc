/* ANSI-C code produced by gperf version 3.0.1 */
/* Command-line: gperf -t -c -Ktext --null-strings --hash-function=KeywordHash --lookup-function=CheckKeyword -G -CP --language=ANSI-C -multiple-iterations=20 keywords.txt  */
/* Computed positions: -k'1,3-5,8' */

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

#line 1 "keywords.txt"
struct Keyword {
	char *		text;
	TokenType	token;
};

#define TOTAL_KEYWORDS 167
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 25
#define MAX_HASH_VALUE 700
/* maximum key range = 676, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
KeywordHash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701,  20,
       95, 209, 250, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701,   0, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701,  25, 701,   0, 215,  20,
       65,  15, 175, 225,   3, 150, 701,  30,  35,  50,
      110, 115,  30, 150,  10,   0,   5, 210, 165,   0,
       45,   0,   5, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701, 701, 701, 701, 701,
      701, 701, 701, 701, 701, 701
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
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

struct stringpool_t
  {
    char stringpool_str25[sizeof("class")];
    char stringpool_str29[sizeof("cast")];
    char stringpool_str34[sizeof("else")];
    char stringpool_str35[sizeof("__swz")];
    char stringpool_str39[sizeof("__swz_sat")];
    char stringpool_str45[sizeof("__str")];
    char stringpool_str50[sizeof("__scs")];
    char stringpool_str51[sizeof("extern")];
    char stringpool_str53[sizeof("asm")];
    char stringpool_str54[sizeof("__scs_sat")];
    char stringpool_str60[sizeof("__ret")];
    char stringpool_str61[sizeof("superp")];
    char stringpool_str69[sizeof("lowp")];
    char stringpool_str70[sizeof("__slt")];
    char stringpool_str75[sizeof("__arl")];
    char stringpool_str80[sizeof("__sle")];
    char stringpool_str85[sizeof("__cal")];
    char stringpool_str88[sizeof("external")];
    char stringpool_str90[sizeof("__rcp")];
    char stringpool_str92[sizeof("discard")];
    char stringpool_str94[sizeof("__rcp_sat")];
    char stringpool_str95[sizeof("__tex")];
    char stringpool_str98[sizeof("__retval")];
    char stringpool_str99[sizeof("__tex_sat")];
    char stringpool_str100[sizeof("__dst")];
    char stringpool_str101[sizeof("packed")];
    char stringpool_str104[sizeof("__dst_sat")];
    char stringpool_str105[sizeof("__lrp")];
    char stringpool_str109[sizeof("__lrp_sat")];
    char stringpool_str110[sizeof("__txp")];
    char stringpool_str114[sizeof("__txp_sat")];
    char stringpool_str115[sizeof("__txl")];
    char stringpool_str119[sizeof("__txl_sat")];
    char stringpool_str120[sizeof("__exp")];
    char stringpool_str122[sizeof("typedef")];
    char stringpool_str123[sizeof("out")];
    char stringpool_str124[sizeof("__exp_sat")];
    char stringpool_str125[sizeof("__max")];
    char stringpool_str126[sizeof("samplerCube")];
    char stringpool_str128[sizeof("__dph")];
    char stringpool_str129[sizeof("__max_sat")];
    char stringpool_str130[sizeof("__cmp")];
    char stringpool_str132[sizeof("__dph_sat")];
    char stringpool_str134[sizeof("__cmp_sat")];
    char stringpool_str135[sizeof("short")];
    char stringpool_str138[sizeof("hvec2")];
    char stringpool_str140[sizeof("const")];
    char stringpool_str141[sizeof("sizeof")];
    char stringpool_str143[sizeof("template")];
    char stringpool_str144[sizeof("sampler1D")];
    char stringpool_str145[sizeof("__mad")];
    char stringpool_str149[sizeof("__mad_sat")];
    char stringpool_str150[sizeof("sampler1DShadow")];
    char stringpool_str152[sizeof("if")];
    char stringpool_str154[sizeof("mat2")];
    char stringpool_str155[sizeof("__sne")];
    char stringpool_str158[sizeof("int")];
    char stringpool_str159[sizeof("this")];
    char stringpool_str160[sizeof("__add")];
    char stringpool_str161[sizeof("static")];
    char stringpool_str164[sizeof("__add_sat")];
    char stringpool_str165[sizeof("__cos")];
    char stringpool_str169[sizeof("__cos_sat")];
    char stringpool_str170[sizeof("__xpd")];
    char stringpool_str174[sizeof("__xpd_sat")];
    char stringpool_str175[sizeof("__pow")];
    char stringpool_str179[sizeof("attribute")];
    char stringpool_str181[sizeof("switch")];
    char stringpool_str184[sizeof("flat")];
    char stringpool_str185[sizeof("__ex2")];
    char stringpool_str188[sizeof("for")];
    char stringpool_str189[sizeof("__ex2_sat")];
    char stringpool_str190[sizeof("__rsq")];
    char stringpool_str194[sizeof("__rsq_sat")];
    char stringpool_str195[sizeof("__seq")];
    char stringpool_str200[sizeof("dvec2")];
    char stringpool_str204[sizeof("namespace")];
    char stringpool_str205[sizeof("while")];
    char stringpool_str209[sizeof("interface")];
    char stringpool_str217[sizeof("half")];
    char stringpool_str219[sizeof("sampler2D")];
    char stringpool_str220[sizeof("__dp2")];
    char stringpool_str223[sizeof("sampler2DRect")];
    char stringpool_str224[sizeof("__dp2_sat")];
    char stringpool_str225[sizeof("sampler2DShadow")];
    char stringpool_str228[sizeof("volatile")];
    char stringpool_str229[sizeof("sampler2DRectShadow")];
    char stringpool_str230[sizeof("false")];
    char stringpool_str234[sizeof("true")];
    char stringpool_str235[sizeof("__frc")];
    char stringpool_str239[sizeof("__frc_sat")];
    char stringpool_str240[sizeof("__sfl")];
    char stringpool_str241[sizeof("return")];
    char stringpool_str245[sizeof("__abs")];
    char stringpool_str246[sizeof("struct")];
    char stringpool_str249[sizeof("__abs_sat")];
    char stringpool_str250[sizeof("__flr")];
    char stringpool_str252[sizeof("hvec3")];
    char stringpool_str254[sizeof("__flr_sat")];
    char stringpool_str255[sizeof("__ssg")];
    char stringpool_str260[sizeof("__sgt")];
    char stringpool_str265[sizeof("break")];
    char stringpool_str266[sizeof("highp")];
    char stringpool_str268[sizeof("mat3")];
    char stringpool_str270[sizeof("__sge")];
    char stringpool_str279[sizeof("enum")];
    char stringpool_str284[sizeof("vec2")];
    char stringpool_str285[sizeof("ivec2")];
    char stringpool_str290[sizeof("__sin")];
    char stringpool_str293[sizeof("hvec4")];
    char stringpool_str294[sizeof("__sin_sat")];
    char stringpool_str295[sizeof("__txb")];
    char stringpool_str299[sizeof("__txb_sat")];
    char stringpool_str300[sizeof("float")];
    char stringpool_str305[sizeof("fixed")];
    char stringpool_str308[sizeof("continue")];
    char stringpool_str309[sizeof("mat4")];
    char stringpool_str310[sizeof("fvec2")];
    char stringpool_str314[sizeof("dvec3")];
    char stringpool_str325[sizeof("__mul")];
    char stringpool_str329[sizeof("__mul_sat")];
    char stringpool_str332[sizeof("varying")];
    char stringpool_str333[sizeof("sampler3D")];
    char stringpool_str334[sizeof("__dp3")];
    char stringpool_str337[sizeof("sampler3DRect")];
    char stringpool_str338[sizeof("__dp3_sat")];
    char stringpool_str339[sizeof("precision")];
    char stringpool_str340[sizeof("__min")];
    char stringpool_str344[sizeof("__min_sat")];
    char stringpool_str349[sizeof("goto")];
    char stringpool_str350[sizeof("bvec2")];
    char stringpool_str355[sizeof("dvec4")];
    char stringpool_str360[sizeof("__mov")];
    char stringpool_str364[sizeof("__mov_sat")];
    char stringpool_str366[sizeof("output")];
    char stringpool_str369[sizeof("bool")];
    char stringpool_str374[sizeof("long")];
    char stringpool_str375[sizeof("__dp4")];
    char stringpool_str379[sizeof("__dp4_sat")];
    char stringpool_str384[sizeof("void")];
    char stringpool_str385[sizeof("__lg2")];
    char stringpool_str389[sizeof("__lg2_sat")];
    char stringpool_str398[sizeof("vec3")];
    char stringpool_str399[sizeof("ivec3")];
    char stringpool_str400[sizeof("input")];
    char stringpool_str405[sizeof("__log")];
    char stringpool_str409[sizeof("__log_sat")];
    char stringpool_str424[sizeof("fvec3")];
    char stringpool_str428[sizeof("noinline")];
    char stringpool_str436[sizeof("public")];
    char stringpool_str439[sizeof("vec4")];
    char stringpool_str440[sizeof("ivec4")];
    char stringpool_str444[sizeof("invariant")];
    char stringpool_str451[sizeof("inline")];
    char stringpool_str455[sizeof("__sub")];
    char stringpool_str457[sizeof("default")];
    char stringpool_str459[sizeof("__sub_sat")];
    char stringpool_str464[sizeof("bvec3")];
    char stringpool_str465[sizeof("fvec4")];
    char stringpool_str482[sizeof("mediump")];
    char stringpool_str485[sizeof("inout")];
    char stringpool_str505[sizeof("bvec4")];
    char stringpool_str531[sizeof("double")];
    char stringpool_str590[sizeof("union")];
    char stringpool_str657[sizeof("uniform")];
    char stringpool_str658[sizeof("unsigned")];
    char stringpool_str700[sizeof("using")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "class",
    "cast",
    "else",
    "__swz",
    "__swz_sat",
    "__str",
    "__scs",
    "extern",
    "asm",
    "__scs_sat",
    "__ret",
    "superp",
    "lowp",
    "__slt",
    "__arl",
    "__sle",
    "__cal",
    "external",
    "__rcp",
    "discard",
    "__rcp_sat",
    "__tex",
    "__retval",
    "__tex_sat",
    "__dst",
    "packed",
    "__dst_sat",
    "__lrp",
    "__lrp_sat",
    "__txp",
    "__txp_sat",
    "__txl",
    "__txl_sat",
    "__exp",
    "typedef",
    "out",
    "__exp_sat",
    "__max",
    "samplerCube",
    "__dph",
    "__max_sat",
    "__cmp",
    "__dph_sat",
    "__cmp_sat",
    "short",
    "hvec2",
    "const",
    "sizeof",
    "template",
    "sampler1D",
    "__mad",
    "__mad_sat",
    "sampler1DShadow",
    "if",
    "mat2",
    "__sne",
    "int",
    "this",
    "__add",
    "static",
    "__add_sat",
    "__cos",
    "__cos_sat",
    "__xpd",
    "__xpd_sat",
    "__pow",
    "attribute",
    "switch",
    "flat",
    "__ex2",
    "for",
    "__ex2_sat",
    "__rsq",
    "__rsq_sat",
    "__seq",
    "dvec2",
    "namespace",
    "while",
    "interface",
    "half",
    "sampler2D",
    "__dp2",
    "sampler2DRect",
    "__dp2_sat",
    "sampler2DShadow",
    "volatile",
    "sampler2DRectShadow",
    "false",
    "true",
    "__frc",
    "__frc_sat",
    "__sfl",
    "return",
    "__abs",
    "struct",
    "__abs_sat",
    "__flr",
    "hvec3",
    "__flr_sat",
    "__ssg",
    "__sgt",
    "break",
    "highp",
    "mat3",
    "__sge",
    "enum",
    "vec2",
    "ivec2",
    "__sin",
    "hvec4",
    "__sin_sat",
    "__txb",
    "__txb_sat",
    "float",
    "fixed",
    "continue",
    "mat4",
    "fvec2",
    "dvec3",
    "__mul",
    "__mul_sat",
    "varying",
    "sampler3D",
    "__dp3",
    "sampler3DRect",
    "__dp3_sat",
    "precision",
    "__min",
    "__min_sat",
    "goto",
    "bvec2",
    "dvec4",
    "__mov",
    "__mov_sat",
    "output",
    "bool",
    "long",
    "__dp4",
    "__dp4_sat",
    "void",
    "__lg2",
    "__lg2_sat",
    "vec3",
    "ivec3",
    "input",
    "__log",
    "__log_sat",
    "fvec3",
    "noinline",
    "public",
    "vec4",
    "ivec4",
    "invariant",
    "inline",
    "__sub",
    "default",
    "__sub_sat",
    "bvec3",
    "fvec4",
    "mediump",
    "inout",
    "bvec4",
    "double",
    "union",
    "uniform",
    "unsigned",
    "using"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct Keyword wordlist[] =
  {
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 33 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str25,TokenTypeError},
    {-1}, {-1}, {-1},
#line 12 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str29,TokenTypeError},
    {-1}, {-1}, {-1}, {-1},
#line 13 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str34,TokenTypeElse},
#line 161 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str35,TokenTypeAsmSWZ},
    {-1}, {-1}, {-1},
#line 162 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str39,TokenTypeAsmSWZ_SAT},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 158 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str45,TokenTypeAsmSTR},
    {-1}, {-1}, {-1}, {-1},
#line 146 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str50,TokenTypeAsmSCS},
#line 58 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str51,TokenTypeError},
    {-1},
#line 7 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str53,TokenTypeError},
#line 147 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str54,TokenTypeAsmSCS_SAT},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 143 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str60,TokenTypeAsmRET},
#line 67 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str61,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 19 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str69,TokenTypeLowPrecision},
#line 155 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str70,TokenTypeAsmSLT},
    {-1}, {-1}, {-1}, {-1},
#line 96 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str75,TokenTypeAsmARL  },
    {-1}, {-1}, {-1}, {-1},
#line 154 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str80,TokenTypeAsmSLE},
    {-1}, {-1}, {-1}, {-1},
#line 101 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str85,TokenTypeAsmCAL},
    {-1}, {-1},
#line 77 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str88,TokenTypeError},
    {-1},
#line 141 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str90,TokenTypeAsmRCP},
    {-1},
#line 70 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str92,TokenTypeDiscard},
    {-1},
#line 142 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str94,TokenTypeAsmRCP_SAT},
#line 163 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str95,TokenTypeAsmTEX},
    {-1}, {-1},
#line 95 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str98,TokenTypeAsmRetval },
#line 164 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str99,TokenTypeAsmTEX_SAT},
#line 114 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str100,TokenTypeAsmDST},
#line 61 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str101,TokenTypeError},
    {-1}, {-1},
#line 115 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str104,TokenTypeAsmDST_SAT},
#line 128 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str105,TokenTypeAsmLRP},
    {-1}, {-1}, {-1},
#line 129 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str109,TokenTypeAsmLRP_SAT},
#line 169 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str110,TokenTypeAsmTXP},
    {-1}, {-1}, {-1},
#line 170 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str114,TokenTypeAsmTXP_SAT},
#line 167 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str115,TokenTypeAsmTXL},
    {-1}, {-1}, {-1},
#line 168 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str119,TokenTypeAsmTXL_SAT},
#line 118 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str120,TokenTypeAsmEXP},
    {-1},
#line 71 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str122,TokenTypeError},
#line 10 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str123,TokenTypeOut},
#line 119 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str124,TokenTypeAsmEXP_SAT},
#line 132 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str125,TokenTypeAsmMAX},
#line 86 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str126,TokenTypeSamplerCube},
    {-1},
#line 112 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str128,TokenTypeAsmDPH},
#line 133 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str129,TokenTypeAsmMAX_SAT},
#line 102 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str130,TokenTypeAsmCMP},
    {-1},
#line 113 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str132,TokenTypeAsmDPH_SAT},
    {-1},
#line 103 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str134,TokenTypeAsmCMP_SAT},
#line 53 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str135,TokenTypeError},
    {-1}, {-1},
#line 45 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str138,TokenTypeError},
    {-1},
#line 34 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str140,TokenTypeConst},
#line 64 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str141,TokenTypeError},
    {-1},
#line 87 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str143,TokenTypeError},
#line 83 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str144,TokenTypeError},
#line 130 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str145,TokenTypeAsmMAD},
    {-1}, {-1}, {-1},
#line 131 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str149,TokenTypeAsmMAD_SAT},
#line 90 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str150,TokenTypeError},
    {-1},
#line 6 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str152,TokenTypeIf},
    {-1},
#line 20 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str154,TokenTypeFloatMat2},
#line 156 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str155,TokenTypeAsmSNE},
    {-1}, {-1},
#line 9 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str158,TokenTypeInt},
#line 23 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str159,TokenTypeError},
#line 99 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str160,TokenTypeAsmADD},
#line 65 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str161,TokenTypeError},
    {-1}, {-1},
#line 100 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str164,TokenTypeAsmADD_SAT},
#line 104 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str165,TokenTypeAsmCOS},
    {-1}, {-1}, {-1},
#line 105 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str169,TokenTypeAsmCOS_SAT},
#line 171 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str170,TokenTypeAsmXPD},
    {-1}, {-1}, {-1},
#line 172 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str174,TokenTypeAsmXPD_SAT},
#line 140 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str175,TokenTypeAsmPOW},
    {-1}, {-1}, {-1},
#line 78 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str179,TokenTypeAttribute},
    {-1},
#line 68 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str181,TokenTypeError},
    {-1}, {-1},
#line 15 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str184,TokenTypeError},
#line 116 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str185,TokenTypeAsmEX2},
    {-1}, {-1},
#line 8 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str188,TokenTypeFor},
#line 117 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str189,TokenTypeAsmEX2_SAT},
#line 144 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str190,TokenTypeAsmRSQ},
    {-1}, {-1}, {-1},
#line 145 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str194,TokenTypeAsmRSQ_SAT},
#line 148 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str195,TokenTypeAsmSEQ},
    {-1}, {-1}, {-1}, {-1},
#line 35 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str200,TokenTypeError},
    {-1}, {-1}, {-1},
#line 81 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str204,TokenTypeError},
#line 56 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str205,TokenTypeWhile},
    {-1}, {-1}, {-1},
#line 79 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str209,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 17 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str217,TokenTypeError},
    {-1},
#line 84 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str219,TokenTypeSampler2D},
#line 106 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str220,TokenTypeAsmDP2},
    {-1}, {-1},
#line 91 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str223,TokenTypeError},
#line 107 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str224,TokenTypeAsmDP2_SAT},
#line 93 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str225,TokenTypeError},
    {-1}, {-1},
#line 89 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str228,TokenTypeError},
#line 92 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str229,TokenTypeError},
#line 38 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str230,TokenTypeFalse},
    {-1}, {-1}, {-1},
#line 24 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str234,TokenTypeTrue,},
#line 122 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str235,TokenTypeAsmFRC},
    {-1}, {-1}, {-1},
#line 123 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str239,TokenTypeAsmFRC_SAT},
#line 149 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str240,TokenTypeAsmSFL},
#line 63 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str241,TokenTypeReturn},
    {-1}, {-1}, {-1},
#line 97 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str245,TokenTypeAsmABS},
#line 66 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str246,TokenTypeStruct},
    {-1}, {-1},
#line 98 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str249,TokenTypeAsmABS_SAT},
#line 120 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str250,TokenTypeAsmFLR},
    {-1},
#line 46 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str252,TokenTypeError},
    {-1},
#line 121 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str254,TokenTypeAsmFLR_SAT},
#line 157 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str255,TokenTypeAsmSSG},
    {-1}, {-1}, {-1}, {-1},
#line 151 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str260,TokenTypeAsmSGT},
    {-1}, {-1}, {-1}, {-1},
#line 29 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str265,TokenTypeBreak},
#line 44 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str266,TokenTypeHighPrecision},
    {-1},
#line 21 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str268,TokenTypeFloatMat3},
    {-1},
#line 150 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str270,TokenTypeAsmSGE},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 14 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str279,TokenTypeError},
    {-1}, {-1}, {-1}, {-1},
#line 25 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str284,TokenTypeFloatVec2},
#line 50 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str285,TokenTypeIntVec2},
    {-1}, {-1}, {-1}, {-1},
#line 152 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str290,TokenTypeAsmSIN},
    {-1}, {-1},
#line 47 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str293,TokenTypeError},
#line 153 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str294,TokenTypeAsmSIN_SAT},
#line 165 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str295,TokenTypeAsmTXB},
    {-1}, {-1}, {-1},
#line 166 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str299,TokenTypeAsmTXB_SAT},
#line 40 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str300,TokenTypeFloat},
    {-1}, {-1}, {-1}, {-1},
#line 39 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str305,TokenTypeError},
    {-1}, {-1},
#line 76 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str308,TokenTypeContinue},
#line 22 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str309,TokenTypeFloatMat4},
#line 41 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str310,TokenTypeError},
    {-1}, {-1}, {-1},
#line 36 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str314,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 138 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str325,TokenTypeAsmMUL},
    {-1}, {-1}, {-1},
#line 139 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str329,TokenTypeAsmMUL_SAT},
    {-1}, {-1},
#line 73 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str332,TokenTypeVarying},
#line 85 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str333,TokenTypeSampler3D},
#line 108 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str334,TokenTypeAsmDP3},
    {-1}, {-1},
#line 94 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str337,TokenTypeError},
#line 109 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str338,TokenTypeAsmDP3_SAT},
#line 82 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str339,TokenTypePrecision},
#line 134 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str340,TokenTypeAsmMIN},
    {-1}, {-1}, {-1},
#line 135 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str344,TokenTypeAsmMIN_SAT},
    {-1}, {-1}, {-1}, {-1},
#line 16 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str349,TokenTypeError},
#line 30 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str350,TokenTypeBoolVec2},
    {-1}, {-1}, {-1}, {-1},
#line 37 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str355,TokenTypeError},
    {-1}, {-1}, {-1}, {-1},
#line 136 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str360,TokenTypeAsmMOV},
    {-1}, {-1}, {-1},
#line 137 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str364,TokenTypeAsmMOV_SAT},
    {-1},
#line 60 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str366,TokenTypeError},
    {-1}, {-1},
#line 11 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str369,TokenTypeBool},
    {-1}, {-1}, {-1}, {-1},
#line 18 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str374,TokenTypeError},
#line 110 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str375,TokenTypeAsmDP4},
    {-1}, {-1}, {-1},
#line 111 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str379,TokenTypeAsmDP4_SAT},
    {-1}, {-1}, {-1}, {-1},
#line 28 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str384,TokenTypeVoid},
#line 124 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str385,TokenTypeAsmLG2},
    {-1}, {-1}, {-1},
#line 125 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str389,TokenTypeAsmLG2_SAT},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 26 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str398,TokenTypeFloatVec3},
#line 51 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str399,TokenTypeIntVec3},
#line 49 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str400,TokenTypeError},
    {-1}, {-1}, {-1}, {-1},
#line 126 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str405,TokenTypeAsmLOG},
    {-1}, {-1}, {-1},
#line 127 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str409,TokenTypeAsmLOG_SAT},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 42 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str424,TokenTypeError},
    {-1}, {-1}, {-1},
#line 75 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str428,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 62 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str436,TokenTypeError},
    {-1}, {-1},
#line 27 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str439,TokenTypeFloatVec4},
#line 52 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str440,TokenTypeIntVec4},
    {-1}, {-1}, {-1},
#line 80 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str444,TokenTypeInvariant},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 59 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str451,TokenTypeError},
    {-1}, {-1}, {-1},
#line 159 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str455,TokenTypeAsmSUB},
    {-1},
#line 69 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str457,TokenTypeError},
    {-1},
#line 160 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str459,TokenTypeAsmSUB_SAT},
    {-1}, {-1}, {-1}, {-1},
#line 31 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str464,TokenTypeBoolVec3},
#line 43 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str465,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 74 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str482,TokenTypeMediumPrecision},
    {-1}, {-1},
#line 48 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str485,TokenTypeInOut},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 32 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str505,TokenTypeBoolVec4},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 57 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str531,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1},
#line 54 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str590,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 72 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str657,TokenTypeUniform},
#line 88 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str658,TokenTypeError},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 55 "keywords.txt"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str700,TokenTypeError}
  };

#ifdef __GNUC__
__inline
#endif
const struct Keyword *
CheckKeyword (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = KeywordHash (str, len);

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
