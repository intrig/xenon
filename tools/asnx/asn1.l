%option noyywrap
%option nounput

WHITESPC  [ \t\r]

%{

// define this to be nothing since its deprecated in C++11
#ifndef _WIN32
#define register
#endif

#define YY_NO_UNISTD_H

#include <iostream>
#include <string>
#include "asnast.h"
#if defined(USING_MESON)
#include "asn1.tab.hh"
#else
#include "parser.hpp"
#endif


using namespace std;

std::string current_string;
int current_line;

int isatty(int) { return 0; }

%}

%%

"[C]"       return BOXC_SYM;
"[S]"       return BOXS_SYM;

{WHITESPC}+ { }
"."      return DOT_SYM;
","      return COMMA_SYM;
"{"      return LEFTBRACE_SYM;
"}"      return RIGHTBRACE_SYM;
"("      return LEFTPAREN_SYM;
")"      return RIGHTPAREN_SYM;
"["      return LEFTBRACKET_SYM;
"]"      return RIGHTBRACKET_SYM;
"<"      return LESSTHAN_SYM;
"-"      return MINUS_SYM;
"::="    return GETS_SYM;
"|"      return BAR_SYM;
";"      return SEMI_COLON_SYM;
TAGS     return TAGS_SYM;
BOOLEAN  return BOOLEAN_SYM;
INTEGER  return INTEGER_SYM;
BIT      return BIT_SYM;
STRING   return STRING_SYM;
OCTET    return OCTET_SYM;
NULL     return NULL_SYM;
SEQUENCE return SEQUENCE_SYM;
OF       return OF_SYM;
SET      return SET_SYM;
IMPLICIT return IMPLICIT_SYM;
EXPLICIT return EXPLICIT_SYM;
AUTOMATIC return AUTOMATIC_SYM;
CHOICE   return CHOICE_SYM;
ANY      return ANY_SYM;
OPTIONAL return OPTIONAL_SYM;
DEFAULT  return DEFAULT_SYM;
COMPONENTS return COMPONENTS_SYM;
CONTAINING return CONTAINING_SYM;
UNIVERSAL return UNIVERSAL_SYM;
APPLICATION return APPLICATION_SYM;
PRIVATE  return PRIVATE_SYM;
TRUE     return TRUE_SYM;
FALSE    return FALSE_SYM;
BEGIN    return BEGIN_SYM;
END      return END_SYM;
DEFINITIONS return DEFINITIONS_SYM;
ENUMERATED return ENUMERATED_SYM;
EXPORTS  return EXPORTS_SYM;
IMPORTS  return IMPORTS_SYM;
REAL     return REAL_SYM;
INCLUDES return INCLUDES_SYM;
MIN      return MIN_SYM;
MAX      return MAX_SYM;
SIZE     return SIZE_SYM;
FROM     return FROM_SYM;
WITH     return WITH_SYM;
COMPONENT return COMPONENT_SYM;
PRESENT  return PRESENT_SYM;
ABSENT   return ABSENT_SYM;
DEFINED  return DEFINED_SYM;
BY       return BY_SYM;
PLUS-INFINITY return PLUS_INFINITY_SYM;
MINUS-INFINITY return MINUS_INFINITY_SYM;

NumericString	return NUMERICSTRING_SYM;
PrintableString	return PRINTABLESTRING_SYM;
BMPString		return BMPSTRING_SYM;
UniversalString	return UNIVERSALSTRING_SYM;
TeletexString	return TELETEXSTRING_SYM;

OPERATION        return OPERATION_SYM;
ARGUMENT         return ARGUMENT_SYM;
RESULT           return RESULT_SYM;
ERRORS           return ERRORS_SYM;
LINKED           return LINKED_SYM;

ERROR            return ERROR_SYM;
PARAMETER        return PARAMETER_SYM;

BIND             return BIND_SYM;
BIND-ERROR       return BINDERROR_SYM;
UNBIND           return UNBIND_SYM;
UNBIND-ERROR     return UNBINDERROR_SYM;

APPLICATION-CONTEXT          return AC_SYM;
APPLICATION-SERVICE-ELEMENTS return ASES_SYM;
REMOTE                       return REMOTE_SYM;
INITIATOR                    return INITIATOR_SYM;
RESPONDER                    return RESPONDER_SYM;
APPLICATION-SERVICE-ELEMENT return ASE_SYM;
OPERATIONS                  return OPERATIONS_SYM;
EXTENSION-ATTRIBUTE          return EXTENSIONATTRIBUTE_SYM;
EXTENSIONS                   return EXTENSIONS_SYM;
CHOSEN                       return CHOSEN_SYM;

EXTENSION                   return EXTENSION_SYM;
CRITICAL                    return CRITICAL_SYM;
FOR                         return FOR_SYM;
SUBMISSION                  return SUBMISSION_SYM;
DELIVERY                    return DELIVERY_SYM;
TRANSFER                    return TRANSFER_SYM;

OBJECT                  return OBJECT_SYM;
PORTS                   return PORTS_SYM;

PORT                      return PORT_SYM;
REFINE                    return REFINE_SYM;
AS                        return AS_SYM;
RECURRING                 return RECURRING_SYM;
VISIBLE                   return VISIBLE_SYM;
PAIRED                    return PAIRED_SYM;

ABSTRACT-BIND             return ABSTRACTBIND_SYM;
TO                        return TO_SYM;

ABSTRACT-UNBIND           return ABSTRACTUNBIND_SYM;

ABSTRACT-ERROR            return ABSTRACTERROR_SYM;

ABSTRACT-OPERATION        return ABSTRACTOPERATION_SYM;

TOKEN                     return TOKEN_SYM;

TOKEN-DATA                return TOKENDATA_SYM;

SECURITY-CATEGORY         return SECURITYCATEGORY_SYM;

ALGORITHM         return ALGORITHM_SYM;
ENCRYPTED         return ENCRYPTED_SYM;
SIGNED            return SIGNED_SYM;
SIGNATURE         return SIGNATURE_SYM;
PROTECTED         return PROTECTED_SYM;

OBJECT-TYPE       return OBJECTTYPE_SYM;
SYNTAX            return SYNTAX_SYM;
ACCESS            return ACCESS_SYM;
STATUS            return STATUS_SYM;
DESCRIPTION       return DESCRIPTION_SYM;
REFERENCE         return REFERENCE_SYM;
INDEX             return INDEX_SYM;
DEFVAL            return DEFVAL_SYM;



\'[0-1]*\'B { 
        current_string = yytext;
        yylval.value = new BinaryString(yytext);
        return BSTRING_SYM;
    }

\'[0-9A-Fa-f]*\'H { 
        current_string = yytext;
        yylval.value = new HexString(yytext);
        return HSTRING_SYM;}

\"([^\"]|"\"\"")*\"  { 
        current_string = yytext;
        yylval.value = new CharString(yytext);
        return CSTRING_SYM;}

[A-Z](-[a-zA-Z0-9]|[a-zA-Z0-9])* { 
        current_string = yytext;
        yylval.name = new Name(yytext);
        return UCASEFIRST_IDENT_SYM;
    }


[a-z](-[a-zA-Z0-9]|[a-zA-Z0-9])* { 
        current_string = yytext;
        yylval.name = new Name(yytext);
        return LCASEFIRST_IDENT_SYM;}

[1-9][0-9]* { /*first digit cannot be zero on multi-digit #'s*/
        current_string = yytext;
        yylval.value = new SignedNumber(yytext);
		return NUMBER_SYM; }

0 {  /*allow zero as first digit on single digit #'s*/
        current_string = "0";
        yylval.value = new SignedNumber("0");
        return NUMBER_SYM;}

"\n" { ++current_line; }

"--"(-[^\-\n]|[^\-\n])*("--"|\n|"-\n") {
        /* eat comments, update line no */ 
            ++current_line;
        }


%%
