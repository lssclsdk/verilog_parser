/*
 *
 * (c) Copyright 1999 - 2019 Verific Design Automation Inc.
 * All rights reserved.
 *
 * This source code belongs to Verific Design Automation Inc.
 * It is considered trade secret and confidential, and is not to be used
 * by parties who have not received written authorization
 * from Verific Design Automation Inc.
 *
 * Only authorized users are allowed to use, copy and modify
 * this software provided that the above copyright notice
 * remains in all copies of this software.
 *
 *
*/

#include <cstring>          // strchr ...
#include <cctype>           // isalpha, etc ...

#include "Visitor.h"        // Visitor base class definition

#include "Array.h"          // Make dynamic array class Array available
#include "Strings.h"        // A string utility/wrapper class
#include "Message.h"        // Make message handlers available, not used in this example

#include "VeriModule.h"     // Definition of a VeriModule and VeriPrimitive
#include "VeriId.h"         // Definitions of all identifier definition tree nodes
#include "VeriExpression.h" // Definitions of all verilog expression tree nodes
#include "VeriModuleItem.h" // Definitions of all verilog module item tree nodes
#include "VeriStatement.h"  // Definitions of all verilog statement tree nodes
#include "VeriMisc.h"       // Definitions of all extraneous verilog tree nodes (ie. range, path, strength, etc...)
#include "VeriConstVal.h"   // Definitions of parse-tree nodes representing constant values in Verilog.
#include "veri_tokens.h"

#ifdef VERIFIC_NAMESPACE
using namespace Verific ;
#endif

/*-----------------------------------------------------------------*/
//                      Constructor / Destructor
/*-----------------------------------------------------------------*/

PrettyPrintVisitor::PrettyPrintVisitor(char *pFileName)
    : _ofs(pFileName, std::ios::out),
      _bFileGood(true),
      _nLevel(0)
{
    if (!_ofs.rdbuf()->is_open()){
        Message::Error(0, "cannot open file ", pFileName) ;
        _bFileGood = false;
    }
}

PrettyPrintVisitor::~PrettyPrintVisitor()
{
    _ofs.close();
    _bFileGood = false;
}

/*-----------------------------------------------------------------*/
//                          Utility Methods
/*-----------------------------------------------------------------*/

// static
const char* PrettyPrintVisitor::PrintLevel(unsigned level)
{
    // 80 spaces
    static const char *pSpaces = "                                                                                " ;
    static const unsigned nTabSize = 4;
    // Don't overflow on the fixed-length spaces constant
    unsigned space = Strings::len(pSpaces) ;
    if ((nTabSize * level) > space) return pSpaces ;
    return pSpaces + space - (nTabSize * level) ;
}

// static
unsigned PrettyPrintVisitor::IsEscapedIdentifier(const char *str)
{
    // Determine if string is an escaped identifier.  If it is,
    // return 1, else, return 0.
    if (!str || !*str) return 0 ;

    // First character should be [a-zA-Z_]
    if (!std::isalpha(*str) && (*str != '_')) return 1 ;

    // Following characters should be [a-zA-Z0-9_$]
    while (*++str) {
        switch (*str) {
        case '_' :
        case '$' : break ;

        default :
            if (!std::isalpha(*str) && !std::isdigit(*str)) return 1 ;
            break;
        }
    }

    return 0 ;  // string is not an escaped identifier
}

// static
void PrettyPrintVisitor::PrintIdentifier(std::ostream &f, const char *str)
{
    if (!str || !*str) { f << "" ; return ; }

    // Is this a hierarchical identifier?
    if (std::strchr(str, ' ')) {
        // This is a hierarchical identifier!  Output appropriately.
        char *pCpy = Strings::save(str) ;
        char *pStart = pCpy ;
        char *sp = std::strchr(pStart, ' ') ;
        do {
            *sp = '\0' ;
            if (IsEscapedIdentifier(pStart)) f << "\\" << pStart << " " ;
            else f << pStart ;
            pStart = sp + 1;
            sp = std::strchr(pStart, ' ') ;
            if (sp) f << "." ;  // Print verilog hierarchical separator
        } while (sp) ;
        // Output last identifier
        f << "." ;
        if (IsEscapedIdentifier(pStart)) f << "\\" << pStart << " " ;
        else f << pStart ;
        Strings::free(pCpy) ;
    } else {
        // This is not a hierarchical identifier!
        if (IsEscapedIdentifier(str)) f << "\\" << str << " " ;
        else f << str ;
    }
}

// static
const char* PrettyPrintVisitor::PrintToken(unsigned veri_token)
{
    // Get a string for a token
    switch (veri_token) {
    case 0 : return "" ; // No token

    case VERI_OPAREN :              return "(" ;
    case VERI_CPAREN :              return ")" ;
    case VERI_MODULUS :             return "%" ;
    case VERI_PLUS :                return "+" ;
    case VERI_MIN :                 return "-" ;
    case VERI_MUL :                 return "*" ;
    case VERI_DIV :                 return "/" ;
    case VERI_GT :                  return ">" ;
    case VERI_LT :                  return "<" ;
    case VERI_GEQ :                 return ">=" ;
    case VERI_LEQ :                 return "<=" ;
    case VERI_CASEEQ :              return "===" ;
    case VERI_CASENEQ :             return "!==" ;
    case VERI_LOGEQ :               return "==" ;
    case VERI_LOGNEQ :              return "!=" ;
    case VERI_LOGNOT :              return "!" ;
    case VERI_LOGAND :              return "&&" ;
    case VERI_LOGOR :               return "||" ;
    case VERI_REDNOT :              return "~" ;
    case VERI_REDAND :              return "&" ;
    case VERI_REDOR :               return "|" ;
    case VERI_REDXOR :              return "^" ;
    case VERI_REDNAND :             return "~&" ;
    case VERI_REDNOR :              return "~|" ;
    case VERI_REDXNOR :             return "~^" ;
    case VERI_LSHIFT :              return "<<" ;
    case VERI_RSHIFT :              return ">>" ;
    case VERI_QUESTION :            return "?" ;
    case VERI_COLON :               return ":" ;
    case VERI_ALLPATH :             return "*>" ;
    case VERI_LEADTO :              return "=>" ;
    case VERI_RIGHTARROW :          return "->" ;

    case VERI_ALWAYS :              return "always" ;
    case VERI_AND :                 return "and" ;
    case VERI_ASSIGN :              return "assign" ;
    case VERI_BEGIN :               return "begin" ;
    case VERI_BUF :                 return "buf" ;
    case VERI_BUFIF0 :              return "bufif0" ;
    case VERI_BUFIF1 :              return "bufif1" ;
    case VERI_CASE :                return "case" ;
    case VERI_CASEX :               return "casex" ;
    case VERI_CASEZ :               return "casez" ;
    case VERI_CMOS :                return "cmos" ;
    case VERI_DEASSIGN :            return "deassign" ;
    case VERI_DEFAULT :             return "default" ;
    case VERI_DEFPARAM :            return "defparam" ;
    case VERI_DISABLE :             return "disable" ;
    case VERI_EDGE :                return "edge" ;
    case VERI_ELSE :                return "else" ;
    case VERI_END :                 return "end" ;
    case VERI_ENDCASE :             return "endcase" ;
    case VERI_ENDFUNCTION :         return "endfunction" ;
    case VERI_ENDMODULE :           return "endmodule" ;
    case VERI_ENDPRIMITIVE :        return "endprimitive" ;
    case VERI_ENDSPECIFY :          return "endspecify" ;
    case VERI_ENDTABLE :            return "endtable" ;
    case VERI_ENDTASK :             return "endtask" ;
    case VERI_EVENT :               return "event" ;
    case VERI_FOR :                 return "for" ;
    case VERI_FORCE :               return "force" ;
    case VERI_FOREVER :             return "forever" ;
    case VERI_FORK :                return "fork" ;
    case VERI_FUNCTION :            return "function" ;
    case VERI_HIGHZ0 :              return "highz0" ;
    case VERI_HIGHZ1 :              return "highz1" ;
    case VERI_IF :                  return "if" ;
    case VERI_IFNONE :              return "ifnone" ;
    case VERI_INITIAL :             return "initial" ;
    case VERI_INOUT :               return "inout" ;
    case VERI_INPUT :               return "input" ;
    case VERI_INTEGER :             return "integer" ;
    case VERI_JOIN :                return "join" ;
    case VERI_LARGE :               return "large" ;
    case VERI_MACROMODULE :         return "macromodule" ;
    case VERI_MEDIUM :              return "medium" ;
    case VERI_MODULE :              return "module" ;
    case VERI_NAND :                return "nand" ;
    case VERI_NEGEDGE :             return "negedge" ;
    case VERI_NMOS :                return "nmos" ;
    case VERI_NOR :                 return "nor" ;
    case VERI_NOT :                 return "not" ;
    case VERI_NOTIF0 :              return "notif0" ;
    case VERI_NOTIF1 :              return "notif1" ;
    case VERI_OR :                  return "or" ;
    case VERI_OUTPUT :              return "output" ;
    case VERI_PARAMETER :           return "parameter" ;
    case VERI_PMOS :                return "pmos" ;
    case VERI_POSEDGE :             return "posedge" ;
    case VERI_PRIMITIVE :           return "primitive" ;
    case VERI_PULL0 :               return "pull0" ;
    case VERI_PULL1 :               return "pull1" ;
    case VERI_PULLDOWN :            return "pulldown" ;
    case VERI_PULLUP :              return "pullup" ;
    case VERI_RCMOS :               return "rcmos" ;
    case VERI_REAL :                return "real" ;
    case VERI_REALTIME :            return "realtime" ;
    case VERI_REG :                 return "reg" ;
    case VERI_RELEASE :             return "release" ;
    case VERI_REPEAT :              return "repeat" ;
    case VERI_RNMOS :               return "rnmos" ;
    case VERI_RPMOS :               return "rpmos" ;
    case VERI_RTRAN :               return "rtran" ;
    case VERI_RTRANIF0 :            return "rtranif0" ;
    case VERI_RTRANIF1 :            return "rtranif1" ;
    case VERI_SCALARED :            return "scalared" ;
    case VERI_SMALL :               return "small" ;
    case VERI_SPECIFY :             return "specify" ;
    case VERI_SPECPARAM :           return "specparam" ;
    case VERI_STRONG0 :             return "strong0" ;
    case VERI_STRONG1 :             return "strong1" ;
    case VERI_SUPPLY0 :             return "supply0" ;
    case VERI_SUPPLY1 :             return "supply1" ;
    case VERI_TABLE :               return "table" ;
    case VERI_TASK :                return "task" ;
    case VERI_TIME :                return "time" ;
    case VERI_TRAN :                return "tran" ;
    case VERI_TRANIF0 :             return "tranif0" ;
    case VERI_TRANIF1 :             return "tranif1" ;
    case VERI_TRI :                 return "tri" ;
    case VERI_TRI0 :                return "tri0" ;
    case VERI_TRI1 :                return "tri1" ;
    case VERI_TRIAND :              return "triand" ;
    case VERI_TRIOR :               return "trior" ;
    case VERI_TRIREG :              return "trireg" ;
    case VERI_VECTORED :            return "vectored" ;
    case VERI_WAIT :                return "wait" ;
    case VERI_WAND :                return "wand" ;
    case VERI_WEAK0 :               return "weak0" ;
    case VERI_WEAK1 :               return "weak1" ;
    case VERI_WHILE :               return "while" ;
    case VERI_WIRE :                return "wire" ;
    case VERI_WOR :                 return "wor" ;
    case VERI_XNOR :                return "xnor" ;
    case VERI_XOR :                 return "xor" ;

    case VERI_OCUR :                return "{" ;
    case VERI_CCUR :                return "}" ;
    case VERI_OBRACK :              return "[" ;
    case VERI_CBRACK :              return "]" ;
    case VERI_SEMI :                return ";" ;
    case VERI_DOT :                 return "." ;
    case VERI_COMMA :               return "," ;
    case VERI_AT :                  return "@" ;
    case VERI_EQUAL :               return "=" ;
    case VERI_POUND :               return "#" ;

    case VERI_EDGE_AMPERSAND :      return "&&&" ;

        // Verilog 2000 added tokens :
    case VERI_PARTSELECT_UP :       return "+:" ;
    case VERI_PARTSELECT_DOWN :     return "-:" ;
    case VERI_ARITLSHIFT :          return "<<<" ;
    case VERI_ARITRSHIFT :          return ">>>" ;
    case VERI_POWER :               return "**" ;
    //case VERI_OP_STAR_CP :          return "(*)" ;
    case VERI_OATTR :               return "(*" ;
    case VERI_CATTR :               return "*)" ;

        // Verilog 2000 added keywords :
    case VERI_CONFIG :              return "config" ;
    case VERI_ENDCONFIG :           return "endconfig" ;
    case VERI_DESIGN :              return "design" ;
    case VERI_INSTANCE :            return "instance" ;
    case VERI_INCDIR :              return "incdir" ;
    case VERI_INCLUDE :             return "include" ;
    case VERI_CELL :                return "cell" ;
    case VERI_USE :                 return "use" ;
    case VERI_LIBRARY :             return "library" ;
    case VERI_LIBLIST :             return "liblist" ;
    case VERI_GENERATE :            return "generate" ;
    case VERI_ENDGENERATE :         return "endgenerate" ;
    case VERI_GENVAR :              return "genvar" ;
    case VERI_LOCALPARAM :          return "localparam" ;
    case VERI_SIGNED :              return "signed" ;
    case VERI_UNSIGNED :            return "unsigned" ;
    case VERI_AUTOMATIC :           return "automatic" ;
    case VERI_PULSESTYLE_ONEVENT :  return "pulsestyle_onevent" ;
    case VERI_PULSESTYLE_ONDETECT : return "pulsestyle_ondetect" ;
    case VERI_SHOWCANCELLED :       return "showcancelled" ;
    case VERI_NOSHOWCANCELLED :     return "noshowcancelled" ;

    // Added for default nettype :
    default : return "" ;
    }
}

/*-----------------------------------------------------------------*/
//          Visit Methods : Class details in VeriTreeNode.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriTreeNode, node)
{
    // Do nothing
}

/*-----------------------------------------------------------------*/
//          Visit Methods : Class details in VeriModule.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriModule, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Start myself on a new line
    //_ofs << "MODULE START " << std::endl;
    _ofs << std::endl ;

    // Print the predefined directives :
    if (node.GetDefaultNetType() && node.GetDefaultNetType()!=VERI_WIRE) { _ofs << "`default_nettype " << PrintToken(node.GetDefaultNetType()) << std::endl ; }
    if (node.IsCellDefine())         { _ofs << "`celldefine" << std::endl ; }
    if (node.GetUnconnectedDrive())  { _ofs << "`unconnected_drive " << PrintToken(node.GetUnconnectedDrive()) << std::endl ; }
    if (node.GetTimeScale())         { _ofs << "`timescale " << node.GetTimeScale() << std::endl ; }

    // Print the indent
    _ofs << PrintLevel(_nLevel) ;

    // Keyword
    if (node.GetId()->IsUdp())  { _ofs << "primitive " ; }
    else                       { _ofs << "module " ; }

    // module name
    node.GetId()->Accept(*this) ;
    _ofs << " " ;

    unsigned i ;

    // list of parameters
    if (node.GetParameterConnects()) {
        _ofs << "#(" ;
        VeriModuleItem *item ;
        FOREACH_ARRAY_ITEM(node.GetParameterConnects(), i, item) {
            if (i) _ofs << ", " ;
            // FIX ME : this prints old-style param decls. Not ANSI-complient ones.
            if (item) { IncTabLevel(1) ; item->Accept(*this) ; DecTabLevel(1) ; }
        }
        _ofs << ") " ;
    }

    // list of ports
    if (node.GetPortConnects()) {
        _ofs << "(" ;
        VeriExpression *pc ;
        FOREACH_ARRAY_ITEM(node.GetPortConnects(), i, pc) {
            if (i) _ofs << ", " ;
            if (pc) { IncTabLevel(1) ; pc->Accept(*this) ; DecTabLevel(1) ; }
        }
        _ofs << ")" ;
    }
    _ofs << " ;" << std::endl ;

    // VeriModule Items
    VeriModuleItem *mi ;
    FOREACH_ARRAY_ITEM(node.GetModuleItems(), i, mi) {
        // Write them with one _nLevel indent deeper
        if (mi) { IncTabLevel(1) ; mi->Accept(*this) ; DecTabLevel(1) ; }
    }

    // Close module
    _ofs << PrintLevel(_nLevel) ;
    _ofs << "endmodule" << std::endl << std::endl ;

    // close flag-directives if we can :
    if (node.IsCellDefine())         _ofs << "`endcelldefine" << std::endl ;
    if (node.GetUnconnectedDrive())  _ofs << "`nounconnected_drive" << std::endl ;

    _ofs << std::endl ; // extra linefeed.
}

void PrettyPrintVisitor::VERI_VISIT(VeriPrimitive, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriModule, static_cast<VeriModule&>(node));
}

/*-----------------------------------------------------------------*/
//          Visit Methods : Class details in VeriStatement.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriStatement, node)
{
    // Do nothing
}

void PrettyPrintVisitor::VERI_VISIT(VeriBlockingAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    // target
    if (node.GetLVal()) node.GetLVal()->Accept(*this) ;
    _ofs << " " << PrintToken(VERI_EQUAL) << " " ;

    // options
    if (node.GetControl()) {
        node.GetControl()->Accept(*this) ;
        _ofs << " " ;
    }

    // value
    if (node.GetValue()) node.GetValue()->Accept(*this) ;

    _ofs << " ;" << std::endl ;
}

// Special method
void PrettyPrintVisitor::PrettyPrintWOSemi(VeriBlockingAssign &node)
{
    if (!_bFileGood) return ; // file stream is not good

    // This routine should only be called by VERI_VISIT(VeriFor, node).
    // We don't want a ";" and endl to be printed.

    _ofs << PrintLevel(_nLevel) ;

    // target
    if (node.GetLVal()) node.GetLVal()->Accept(*this) ;
    _ofs << " " << PrintToken(VERI_EQUAL) << " " ;

    // options
    if (node.GetControl()) {
        node.GetControl()->Accept(*this) ;
        _ofs << " " ;
    }

    // value
    if (node.GetValue()) node.GetValue()->Accept(*this) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriNonBlockingAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    // target
    if (node.GetLVal()) node.GetLVal()->Accept(*this) ;
    _ofs << " " << PrintToken(VERI_LEQ) << " " ;

    // options
    if (node.GetControl()) {
        node.GetControl()->Accept(*this) ;
        _ofs << " " ;
    }

    // separate options and value
    _ofs << " ";

    // value
    if (node.GetValue()) node.GetValue()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenVarAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    // target
    PrintIdentifier(_ofs, (node.GetName()) ? node.GetName() : node.GetId()->GetName()) ;
    _ofs << " " << PrintToken(VERI_EQUAL) << " " ;

    // value
    if (node.GetValue()) node.GetValue()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_ASSIGN) << " " ;
    if (node.GetAssign()) node.GetAssign()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriDeAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_DEASSIGN) << " " ;
    if (node.GetLVal()) node.GetLVal()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriForce, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_FORCE) << " " ;
    if (node.GetAssign()) node.GetAssign()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriRelease, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_RELEASE) << " " ;
    if (node.GetLVal()) node.GetLVal()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriTaskEnable, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    if (node.GetTaskName()) node.GetTaskName()->Accept(*this) ;
    _ofs << " " ;

    if (node.GetArgs()) {
        _ofs << "(" ;
        unsigned i ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetArgs(), i, expr) {
            if (i) _ofs << "," ;
            if (!expr) continue ;
            expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriSystemTaskEnable, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    // PrintIdentifier is not called here, since system call names can't be escaped.or hierarchical
    _ofs << "$" << node.GetName() << " " ;

    if (node.GetArgs()) {
        _ofs << "(" ;
        unsigned i ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetArgs(), i, expr) {
            if (i) _ofs << "," ;
            if (!expr) continue ;
            expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriDelayControlStatement, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetDelay()) {
        _ofs << PrintLevel(_nLevel) ;
        // Its one expression. Not a normal delay structure
        _ofs << "#" ;
        node.GetDelay()->Accept(*this) ;
        _ofs << " ";
    }
    if (node.GetStmt()) node.GetStmt()->Accept(*this) ;
    else _ofs << ";" << std::endl;
}

void PrettyPrintVisitor::VERI_VISIT(VeriEventControlStatement, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetAt()) {
        _ofs << PrintLevel(_nLevel) ;
        _ofs << "@(" ;

        // 'or' separated list of expressions
        if (!node.GetAt() || node.GetAt()->Size() == 0) {
            // Implicit event_expression list
            _ofs << "*" ;
        } else {
            unsigned i ;
            VeriExpression *event_expr ;
            FOREACH_ARRAY_ITEM(node.GetAt(), i, event_expr) {
                if (i) _ofs << " or " ;
                if (event_expr) event_expr->Accept(*this) ;
            }
        }
        _ofs << ")" ;
        if (node.GetStmt()) _ofs << std::endl ;
    }

    if (node.GetStmt()) node.GetStmt()->Accept(*this) ;
    else _ofs << ";" << std::endl;
}

void PrettyPrintVisitor::VERI_VISIT(VeriConditionalStatement, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_IF) << " (" ;
    if (node.GetIfExpr()) node.GetIfExpr()->Accept(*this) ;
    _ofs << ") " << std::endl ; // statements start on a new line
    if (node.GetThenStmt()) { IncTabLevel(1) ; node.GetThenStmt()->Accept(*this) ; DecTabLevel(1) ; }

    if (node.GetElseStmt()) {
        _ofs << PrintLevel(_nLevel) << "else" << std::endl ;
        IncTabLevel(1) ; node.GetElseStmt()->Accept(*this) ; DecTabLevel(1) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriCaseStatement, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(node.GetCaseStyle()) << " (" ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ;
    _ofs << ")" << std::endl ;

    // Print Synopsys pragmas if needed
    if (node.IsFullCase() || node.IsParallelCase()) {
        _ofs << PrintLevel(_nLevel) << "// synopsys" ;
        if (node.IsFullCase()) _ofs << " full_case" ;
        if (node.IsParallelCase()) _ofs << " parallel_case" ;
        _ofs << std::endl ;
    }

    if (node.GetCaseItems()) {
        IncTabLevel(1) ;
        unsigned i ;
        VeriCaseItem *ci ;
        FOREACH_ARRAY_ITEM(node.GetCaseItems(), i, ci) {
            VERI_VISIT_NODE(VeriCaseItem, *ci) ;
        }
        DecTabLevel(1) ;
    }

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_ENDCASE) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriForever, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_FOREVER) << std::endl ;
    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; }
}

void PrettyPrintVisitor::VERI_VISIT(VeriRepeat, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_REPEAT) << " (" ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ;
    _ofs << ")" << std::endl ;
    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; } ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriWhile, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_WHILE) << " (" ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ;
    _ofs << ")" << std::endl ;
    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; }
}

void PrettyPrintVisitor::VERI_VISIT(VeriFor, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_FOR) << " (" ;

    // NOTE: Both node.GetInitial() and node.GetRepetition() will always be VeriBlockingAssign's!

    // Must call this special pretty print function so that semicolons and
    // endl's don't get printed.
    unsigned i ;
    VeriModuleItem *item ;
    FOREACH_ARRAY_ITEM(node.GetInitials(), i, item) {
        // a comma-separated list of assignments
        if (i) _ofs << ", " ;
        // This cast is only good for Verilog 95/2000.
        // For SystemVerilog, there can be declarations (VeriModuleItems) here too.
        // VIPER #4091 : If initialization is data decl, treat that separetly
        if (item && item->IsDataDecl()) {
            PrettyPrintWOSemi(*static_cast<VeriDataDecl*>(item)) ;
        } else {
            PrettyPrintWOSemi(*static_cast<VeriBlockingAssign*>(item)) ;
        }
    }
    _ofs << " ; " ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ;
    _ofs << " ; " ;
    // Must call this special pretty print function so that semicolons and
    // endl's don't get printed.
    FOREACH_ARRAY_ITEM(node.GetRepetitions(), i, item) {
        // a comma-separated list of assignments
        if (i) _ofs << ", " ;
        PrettyPrintWOSemi(*static_cast<VeriBlockingAssign*>(item)) ;
    }
    _ofs << ")" << std::endl ;

    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; }
}

void PrettyPrintVisitor::VERI_VISIT(VeriWait, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_WAIT) << " (" ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ; ;
    _ofs << ") " ;
    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; }
    else _ofs << ";" << std::endl;
}

void PrettyPrintVisitor::VERI_VISIT(VeriDisable, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_DISABLE) << " " ;
    if (node.GetTaskBlockName()) node.GetTaskBlockName()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriEventTrigger, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_RIGHTARROW) << " " ;
    if (node.GetEventName()) node.GetEventName()->Accept(*this) ;
    _ofs << " ;" << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriSeqBlock, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_BEGIN) ;

    if (node.GetLabel()) {
        _ofs << " : " ; node.GetLabel()->Accept(*this) ;
    }
    _ofs << std::endl ;

    // Print block items one level deeper :
    IncTabLevel(1) ;
    unsigned i ;
    VeriModuleItem *decl ;
    FOREACH_ARRAY_ITEM(node.GetDeclItems(), i, decl) {
        if (decl) decl->Accept(*this) ;
    }
    DecTabLevel(1) ;

    VeriStatement *stmt ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetStatements(), i, stmt) {
        // VIPER #4281: Add the null-check for 'stmt' otherwise we crash for null statements:
        if (stmt) stmt->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_END) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriParBlock, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_FORK) ;

    if (node.GetLabel()) {
        _ofs << " : " ; node.GetLabel()->Accept(*this) ;
    }
    _ofs << std::endl ;

    // Print block items one level deeper :
    IncTabLevel(1) ;
    unsigned i ;
    VeriModuleItem *decl ;
    FOREACH_ARRAY_ITEM(node.GetDeclItems(), i, decl) {
        if (decl) decl->Accept(*this) ;
    }
    DecTabLevel(1) ;

    VeriStatement *stmt ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetStatements(), i, stmt) {
        stmt->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_JOIN) << std::endl ;
}

/*-----------------------------------------------------------------*/
//        Visit Methods : Class details in VeriModuleItem.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriModuleItem, node)
{
    // Do nothing
}

void PrettyPrintVisitor::VERI_VISIT(VeriDataDecl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Print direction (if there) :
    if (node.GetDir()) _ofs << PrintToken(node.GetDir()) << " " ;

    // Print the style of declaration (if applicable) :
    switch (node.GetDeclType()) {
    case VERI_PARAMETER :
    case VERI_LOCALPARAM :
    case VERI_SPECPARAM :
    case VERI_GENVAR :
         //_ofs << "parameter declaration" << std::endl;
         _ofs << PrintToken(node.GetDeclType()) << " " ;
         break ;
    default : break ; // Other declarations (io,reg etc) only print data type..
    }

    // Print data type (if there) :
    if (node.GetDataType()) node.GetDataType()->Accept(*this) ;

    unsigned i ;
    VeriIdDef *id ;
    FOREACH_ARRAY_ITEM(node.GetIds(), i, id) {
        if (!id) continue ;
        if (i) {
            _ofs << "," << std::endl ;
            _ofs << PrintLevel(_nLevel+1) ;
        }
        id->Accept(*this) ;
    }

    // _ofs << " ; " << std::endl ;

    switch (node.GetDeclType()) {
    case VERI_PARAMETER :
    case VERI_LOCALPARAM :
    case VERI_SPECPARAM :
    case VERI_GENVAR :
         _ofs << std::endl ;
         break ;
    default : _ofs << " ; " << std::endl ;
    }
}

// VIPER #4091 : Special routine :
void PrettyPrintVisitor::PrettyPrintWOSemi(VeriDataDecl &node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Print direction (if there) :
    if (node.GetDir()) _ofs << PrintToken(node.GetDir()) << " " ;

    // Print the style of declaration (if applicable) :
    switch (node.GetDeclType()) {
    case VERI_PARAMETER :
    case VERI_LOCALPARAM :
    case VERI_SPECPARAM :
    case VERI_GENVAR :
         _ofs << PrintToken(node.GetDeclType()) << " " ;
         break ;
    default : break ; // Other declarations (io,reg etc) only print data type..
    }

    // Print data type (if there) :
    if (node.GetDataType()) node.GetDataType()->Accept(*this) ;

    unsigned i ;
    VeriIdDef *id ;
    FOREACH_ARRAY_ITEM(node.GetIds(), i, id) {
        if (!id) continue ;
        if (i) {
            _ofs << "," << std::endl ;
            _ofs << PrintLevel(_nLevel+1) ;
        }
        id->Accept(*this) ;
    }
}
void PrettyPrintVisitor::VERI_VISIT(VeriNetDecl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(node.GetNetType()) << " " ;

    if (node.GetSignedType())    { _ofs << PrintToken(node.GetSignedType()) ; }
    if (node.GetStrength())      { node.GetStrength()->Accept(*this) ; }
    if (node.GetRange())         {
        _ofs << "[" ;
        node.GetRange()->Accept(*this) ;
        _ofs << "] "  ;
    }

    unsigned i ;
    if (node.GetDelay()) {
        _ofs << "#(" ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetDelay(), i, expr) {
            if (i) _ofs << "," ;
            if (expr) expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }

    VeriTreeNode *net_decl ;
    FOREACH_ARRAY_ITEM(node.GetIds(), i, net_decl) {
        if (i) _ofs << ", " ;
        net_decl->Accept(*this) ;
    }
    _ofs << " ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriFunctionDecl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_FUNCTION) << " " ;

    // Print the data type (if there)
    if (node.GetDataType()) node.GetDataType()->Accept(*this) ;

    // Now the declared identifier :
    node.GetFunctionId()->Accept(*this) ;
    _ofs << " ; " << std::endl ;

    // Now decls and statement
    // Indent increment
    IncTabLevel(1) ;

    unsigned i ;
    VeriModuleItem *decl_item ;
    FOREACH_ARRAY_ITEM(node.GetDeclarations(), i, decl_item) {
        if (decl_item) decl_item->Accept(*this) ;
    }
    VeriStatement *stmt ;
    FOREACH_ARRAY_ITEM(node.GetStatements(), i, stmt) {
        if (stmt) stmt->Accept(*this) ;
    }

    // If there were no statements, print a single semicolon, for legal Verilog
    if (!node.GetStatements()) _ofs << PrintLevel(_nLevel) << "; // NOOP" << std::endl ;

    // indent decrement
    DecTabLevel(1) ;

    _ofs << PrintLevel(_nLevel) << PrintToken(VERI_ENDFUNCTION) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriTaskDecl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_TASK) << " " ;

    node.GetTaskId()->Accept(*this) ;
    _ofs << " ; " << std::endl ;

    // Now decls and statement
    unsigned i ;
    VeriModuleItem *decl_item ;

    // Indent increment
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetDeclarations(), i, decl_item) {
        decl_item->Accept(*this) ;
    }
    VeriStatement *stmt ;
    FOREACH_ARRAY_ITEM(node.GetStatements(), i, stmt) {
        if (stmt) stmt->Accept(*this) ;
    }

    // If there were no statements, print a single semicolon, for legal Verilog
    if (!node.GetStatements()) _ofs << PrintLevel(_nLevel) << "; // NOOP" << std::endl ;

    // indent decrement
    DecTabLevel(1) ;

    _ofs << PrintLevel(_nLevel) << PrintToken(VERI_ENDTASK) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriDefParam, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_DEFPARAM) << " " ;

    unsigned i ;
    VeriIdDef *id ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetIds(), i, id) {
        if (i) {
            _ofs << "," << std::endl ;
            _ofs << PrintLevel(_nLevel) ;
        }
        id->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << " ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriContinuousAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_ASSIGN) << " " ;

    if (node.GetStrength()) node.GetStrength()->Accept(*this) ;

    unsigned i ;
    if (node.GetDelay()) {
        _ofs << "#(" ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetDelay(), i, expr) {
            if (i) _ofs << "," ;
            if (expr) expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }

    VeriNetRegAssign *assign ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetNetAssigns(), i, assign) {
        if (i) {
            _ofs << "," << std::endl ;
            _ofs << PrintLevel(_nLevel) ;
        }
        assign->Accept(*this) ;
    }
    DecTabLevel(1) ;

    _ofs << " ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriGateInstantiation, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(node.GetInstType()) << " " ;

    if (node.GetStrength()) node.GetStrength()->Accept(*this) ;

    unsigned i ;
    if (node.GetDelay()) {
        _ofs << "#(" ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetDelay(), i, expr) {
            if (i) _ofs << "," ;
            if (expr) expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }

    VeriInstId *inst ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetInstances(), i, inst) {
        if (i) {
            _ofs << "," << std::endl ;
            _ofs << PrintLevel(_nLevel) ;
        }
        inst->Accept(*this) ;
    }
    DecTabLevel(1) ;

    _ofs << " ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriModuleInstantiation, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    PrintIdentifier(_ofs, node.GetModuleName()) ;
    _ofs << " " ;

    if (node.GetStrength()) node.GetStrength()->Accept(*this) ;

    unsigned i ;
    if (node.GetParamValues()) {
        _ofs << "#(" ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetParamValues(), i, expr) {
            if (i) _ofs << "," ;
            if (expr) expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }

    VeriInstId *inst ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetInstances(), i, inst) {
        if (i) {
            _ofs << "," << std::endl ;
            _ofs << PrintLevel(_nLevel) ;
        }
        inst->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << " ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriSpecifyBlock, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_SPECIFY) << std::endl ;

    VeriModuleItem *mi ;
    unsigned i ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetSpecifyItems(), i, mi) {
        mi->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << PrintLevel(_nLevel) << PrintToken(VERI_ENDSPECIFY) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriPathDecl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    if (node.GetCondition()) {    // state-dependent path
        _ofs << "if (";
        node.GetCondition()->Accept(*this) ;
        _ofs << ") " ;
    } else if (node.IsIfNone()) {   // ifnone statement
        _ofs << PrintToken(VERI_IFNONE) << " " ;
    }

    // Print path declaration
    if (node.GetPath()) node.GetPath()->Accept(*this) ;

    _ofs << PrintToken(VERI_EQUAL) << " (" ;

    // Print delays
    unsigned i ;
    VeriExpression *expr ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetDelay(), i, expr) {
        if (i) _ofs << "," ;
        expr->Accept(*this) ;
    }
    DecTabLevel(1) ;

    _ofs << ") ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriSystemTimingCheck, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    // System task names can't be escaped or hierarchical, so don't call VeriNode::PrintIdentifier here.
    _ofs << "$" << node.GetTaskName() << " " ;

    if (node.GetArgs()) {
        _ofs << "(" ;
        unsigned i ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetArgs(), i, expr) {
            if (i) _ofs << "," ;
            if (!expr) continue ;
            expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }
    _ofs << " ; " << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriInitialConstruct, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_INITIAL) << std::endl ;
    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; }
}

void PrettyPrintVisitor::VERI_VISIT(VeriAlwaysConstruct, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_ALWAYS) << std::endl ;
    if (node.GetStmt()) { IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ; }
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenerateConstruct, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_GENERATE) << std::endl ;
    unsigned i ;
    VeriModuleItem *item ;
    FOREACH_ARRAY_ITEM(node.GetItems(), i, item) {
        if (item) { IncTabLevel(1) ; item->Accept(*this) ; DecTabLevel(1) ; }
    }
    _ofs << PrintLevel(_nLevel) << PrintToken(VERI_ENDGENERATE) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenerateConditional, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(VERI_IF) << " (" ;
    node.GetIfExpr()->Accept(*this) ;
    _ofs << ") " << std::endl ; // statements start on a new line
    if (node.GetThenItem()) { IncTabLevel(1) ; node.GetThenItem()->Accept(*this) ; DecTabLevel(1) ; }

    if (node.GetElseItem()) {
        _ofs << PrintLevel(_nLevel) << "else" << std::endl ;
        IncTabLevel(1) ;
        node.GetElseItem()->Accept(*this) ;
        DecTabLevel(1) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenerateCase, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    _ofs << PrintToken(node.GetCaseStyle()) << " (" ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ;
    _ofs << ")" << std::endl ;

    if (node.GetCaseItems()) {
        IncTabLevel(1) ;
        unsigned i ;
        VeriGenerateCaseItem *ci ;
        FOREACH_ARRAY_ITEM(node.GetCaseItems(), i, ci) {
            VERI_VISIT_NODE(VeriGenerateCaseItem, *ci) ;
        }
        DecTabLevel(1) ;
    }

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_ENDCASE) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenerateFor, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;

    // Print 'for' items on one line : FIX ME : printing too many ';'s.
    _ofs << PrintToken(VERI_FOR) << " (" ;
    if (node.GetInitial()) node.GetInitial()->Accept(*this) ;
    _ofs << ";" ;
    if (node.GetCondition()) node.GetCondition()->Accept(*this) ;
    _ofs << ";" ;
    if (node.GetRepetition()) node.GetRepetition()->Accept(*this) ;
    _ofs << ")" << std::endl ;

    // Print the generate block :
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_BEGIN) ;

    if (node.GetBlockId()) {
        _ofs << " : " ; node.GetBlockId()->Accept(*this) ;
    }
    _ofs << std::endl ;

    unsigned i ;
    VeriModuleItem *item ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetItems(), i, item) {
        item->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_END) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenerateBlock, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_BEGIN) ;

    if (node.GetBlockId()) {
        _ofs << " : " ; node.GetBlockId()->Accept(*this) ;
    }
    _ofs << std::endl ;

    unsigned i ;
    VeriModuleItem *item ;
    IncTabLevel(1) ;
    FOREACH_ARRAY_ITEM(node.GetItems(), i, item) {
        item->Accept(*this) ;
    }
    DecTabLevel(1) ;
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_END) << std::endl ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriTable, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(VERI_TABLE) << std::endl ;

    unsigned i ;
    const char *entry ;
    FOREACH_ARRAY_ITEM(node.GetTableEntries(), i, entry) {
        _ofs << PrintLevel(_nLevel) ;
        _ofs << entry << " ;" << std::endl ;
    }

    _ofs << PrintLevel(_nLevel) << PrintToken(VERI_ENDTABLE) << std::endl ;
}

/*-----------------------------------------------------------------*/
//            Visit Methods : Class details in VeriMisc.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriStrength, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << "(" << PrintToken(node.GetLVal()) ;
    if (node.GetRVal()) {
        // If rval is not there, this will be a charge_value or single drive_value
        _ofs << "," << PrintToken(node.GetRVal()) ;
    }
    _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriNetRegAssign, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetLValExpr()) node.GetLValExpr()->Accept(*this) ;
    if (node.GetRValExpr()) {
        _ofs << " " << PrintToken(VERI_EQUAL) << " " ;
        node.GetRValExpr()->Accept(*this) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriInstId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetName()) { _ofs << node.GetName() << " " ; }
    if (node.GetRange()) {
        _ofs << "[" ;
        node.GetRange()->Accept(*this) ;
        _ofs << "] " ;
    }
    // Port connection list is always there. Even when empty
    _ofs << "(" ;
    unsigned i ;
    VeriExpression *pc ;
    FOREACH_ARRAY_ITEM(node.GetPortConnects(), i, pc) {
        if (i) _ofs << ", " ;
        if (!pc) continue ;
        pc->Accept(*this) ;
    }
    _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriCaseItem, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Each item starts at a new line
    _ofs << PrintLevel(_nLevel) ;

    if (node.GetConditions()) {
        unsigned i ;
        VeriExpression *condition ;
        FOREACH_ARRAY_ITEM(node.GetConditions(), i, condition) {
            if (i) _ofs << ", " ;
            if (condition) condition->Accept(*this) ;
        }
    } else {
        // 'default' case
        _ofs << PrintToken(VERI_DEFAULT) ;
    }
    // Print statement on new line, one level deeper
    _ofs << " : " << std::endl ;
    if (node.GetStmt()) {
        IncTabLevel(1) ; node.GetStmt()->Accept(*this) ; DecTabLevel(1) ;
        // VeriStatement already ends with a newline
    } else {
        // Print semicolon only (null), and end with a newline (as a normal statement)
        _ofs << PrintLevel(_nLevel+1) << "; " << std::endl ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenerateCaseItem, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Each item starts at a new line
    _ofs << PrintLevel(_nLevel) ;

    if (node.GetConditions()) {
        unsigned i ;
        VeriExpression *condition ;
        FOREACH_ARRAY_ITEM(node.GetConditions(), i, condition) {
            if (i) _ofs << ", " ;
            if (condition) condition->Accept(*this) ;
        }
    } else {
        // 'default' case
        _ofs << PrintToken(VERI_DEFAULT) ;
    }
    // Print statement on new line, one level deeper
    _ofs << " : " << std::endl ;
    if (node.GetItem()) {
        IncTabLevel(1) ; node.GetItem()->Accept(*this) ; DecTabLevel(1) ;
        // VeriStatement already ends with a newline
    } else {
        // Print semicolon only (null), and end with a newline (as a normal statement)
        _ofs << PrintLevel(_nLevel+1) << "; " << std::endl ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriPath, node)
{
    if (!_bFileGood) return ; // file stream is not good

    unsigned i ;
    VeriExpression *expr ;

    int bSimplePath = (node.GetDataSource()) ? 0 : 1 ;
    int bComplexPath = !bSimplePath ;

    _ofs << "(" ;
    if (bComplexPath && node.GetEdge()) _ofs << PrintToken(node.GetEdge()) << " ";
    FOREACH_ARRAY_ITEM(node.GetInTerminals(), i, expr) {
        if (i) _ofs << "," ;
        if (expr) expr->Accept(*this) ;
    }

    _ofs << " " ;
    if (bSimplePath && node.GetPolarityOperator()) _ofs << PrintToken(node.GetPolarityOperator()) ;
    _ofs << PrintToken(node.GetPathToken()) << " " ;

    if (bComplexPath) _ofs << "(" ;
    FOREACH_ARRAY_ITEM(node.GetOutTerminals(), i, expr) {
        if (i) _ofs << "," ;
        if (expr) expr->Accept(*this) ;
    }

    if (bComplexPath) {
        _ofs << " " ;
        if (node.GetPolarityOperator()) _ofs << PrintToken(node.GetPolarityOperator()) ;
        _ofs << ": ";
        node.GetDataSource()->Accept(*this) ;
        _ofs << ")" ;
    }

    _ofs << ") " ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriDelayOrEventControl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetDelayControl()) {
        _ofs << "#" ;
        node.GetDelayControl()->Accept(*this) ;
    }
    if (node.GetEventControl()) {
        if (node.GetRepeatEvent()) {
            _ofs << "repeat (" ;
            node.GetRepeatEvent()->Accept(*this) ;
            _ofs << ") " ;
        }
        _ofs << "@ (" ;

        // 'or' separated list of expressions
        unsigned i ;
        VeriExpression *event_expr ;
        FOREACH_ARRAY_ITEM(node.GetEventControl(), i, event_expr) {
            if (i) _ofs << " or " ;
            if (event_expr) event_expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }
}

/*-----------------------------------------------------------------*/
//              Visit Methods : Class details in VeriId.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriIdDef, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // NOTE: The following function VeriNode::PrintIdentifier is now the primary
    // way to pretty print identifiers.  This function does a hierarchical check
    // and escaped identifier check, and prints accordingly.
    PrintIdentifier(_ofs, node.GetName()) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriVariable, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Print the id name with optional initial value and dimensions
    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ; // Print node.GetName()
    if (node.GetDimensions()) {
        _ofs << " " ;
        _ofs << "[" ;
        node.GetDimensions()->Accept(*this) ;
        _ofs << "]" ;
    }
    if (node.GetInitialValue()) {
        _ofs << " " << PrintToken(VERI_EQUAL) << " " ;
        node.GetInitialValue()->Accept(*this) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriModuleId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriUdpId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriTaskId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriFunctionId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriGenVarId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriBlockId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriParamId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Print the id name with initial value
    VERI_VISIT_NODE(VeriIdDef, static_cast<VeriIdDef&>(node)) ;
    if (node.GetInitialValue()) {
        _ofs << " " << PrintToken(VERI_EQUAL) << " " ;
        node.GetInitialValue()->Accept(*this) ;
    }
}

/*-----------------------------------------------------------------*/
//        Visit Methods : Class details in VeriExpression.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriExpression, node)
{
    // Do nothing
}

void PrettyPrintVisitor::VERI_VISIT(VeriIdRef, node)
{
    if (!_bFileGood) return ; // file stream is not good

    PrintIdentifier(_ofs, (node.GetName()) ? node.GetName() : node.FullId()->GetName()) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriIndexedId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetPrefix()) node.GetPrefix()->Accept(*this) ;

    _ofs << "[" ;
    node.GetIndexExpr()->Accept(*this) ;
    _ofs << "]" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriSelectedName, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetPrefix()) node.GetPrefix()->Accept(*this) ;

    _ofs << "." ;
    PrintIdentifier(_ofs, node.GetSuffix()) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriIndexedMemoryId, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetPrefix()) node.GetPrefix()->Accept(*this) ;

    unsigned i ;
    VeriExpression *index ;
    FOREACH_ARRAY_ITEM(node.GetIndexes(), i, index) {
        _ofs << "[" ;
        index->Accept(*this) ;
        _ofs << "]" ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriConcat, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << "{" ;
    unsigned i ;
    VeriExpression *expr ;
    FOREACH_ARRAY_ITEM(node.GetExpressions(), i, expr) {
        if (i) _ofs << "," ;
        if (!expr) continue ;
        expr->Accept(*this);
    }
    _ofs << "}" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriMultiConcat, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << "{" ;
    node.GetRepeat()->Accept(*this) ;
    _ofs << "{" ;
    unsigned i ;
    VeriExpression *expr ;
    FOREACH_ARRAY_ITEM(node.GetExpressions(), i, expr) {
        if (i) _ofs << "," ;
        if (!expr) continue ;
        if (expr) expr->Accept(*this) ;
    }
    _ofs << "}}" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriFunctionCall, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetFunctionName()) node.GetFunctionName()->Accept(*this) ;

    if (node.GetArgs()) {
        _ofs << "(" ;
        unsigned i ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetArgs(), i, expr) {
            if (i) _ofs << "," ;
            if (!expr) continue ;
            expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriSystemFunctionCall, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // System function call names can't be escaped or hierarchical, so don't call VeriNode::PrintIdentifier.
    _ofs << "$" << node.GetName() ;
    if (node.GetArgs()) {
        _ofs << "(" ;
        unsigned i ;
        VeriExpression *expr ;
        FOREACH_ARRAY_ITEM(node.GetArgs(), i, expr) {
            if (i) _ofs << "," ;
            if (!expr) continue ;
            expr->Accept(*this) ;
        }
        _ofs << ")" ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriMinTypMaxExpr, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << "(" ;
    if (node.GetMinExpr()) node.GetMinExpr()->Accept(*this) ;
    _ofs << ":" ;
    if (node.GetTypExpr()) node.GetTypExpr()->Accept(*this) ;
    _ofs << ":" ;
    if (node.GetMaxExpr()) node.GetMaxExpr()->Accept(*this) ;
    _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriUnaryOperator, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintToken(node.OperType()) ;
    // Parenthesize the result to get precedence right
    _ofs << "(" ;
    if (node.GetArg()) node.GetArg()->Accept(*this) ;
    _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriBinaryOperator, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Parenthesize the result to get precedence right
    _ofs << "(" ;
    if (node.GetLeft()) node.GetLeft()->Accept(*this) ;
    _ofs << " " << PrintToken(node.OperType()) << " " ;
    if (node.GetRight()) node.GetRight()->Accept(*this) ;
    _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriQuestionColon, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Parenthesize the result to get precedence right
    _ofs << "(" ;
    if (node.GetIfExpr()) node.GetIfExpr()->Accept(*this) ;
    _ofs << " ? " ;
    if (node.GetThenExpr()) node.GetThenExpr()->Accept(*this) ;
    _ofs << " : " ;
    if (node.GetElseExpr()) node.GetElseExpr()->Accept(*this) ;
    _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriEventExpression, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << PrintToken(node.GetEdgeToken()) << " " ;
    if (node.GetExpr()) node.GetExpr()->Accept(*this) ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriPortConnect, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetNamedFormal()) {
        _ofs << "." ; PrintIdentifier(_ofs, node.GetNamedFormal()) ; _ofs << "(" ;
    }
    if (node.GetConnection()) {
        node.GetConnection()->Accept(*this) ;
    }
    // named connection always has ()
    if (node.GetNamedFormal()) _ofs << ")" ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriPortOpen, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Verilog prints nothing for an open port
}

void PrettyPrintVisitor::VERI_VISIT(VeriAnsiPortDecl, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Start on a newline :
    _ofs << std::endl ;
    _ofs << PrintLevel(_nLevel) ;
    _ofs << PrintToken(node.GetDir()) << " " ;

    // Print the data type :
    if (node.GetDataType()) node.GetDataType()->Accept(*this) ;

    unsigned i ;
    unsigned first = 1 ;
    VeriIdDef *id ;
    FOREACH_ARRAY_ITEM(node.GetIds(), i, id) {
        if (!first) _ofs << ", " ;
        id->Accept(*this) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriTimingCheckEvent, node)
{
    if (!_bFileGood) return ; // file stream is not good

    if (node.GetEdgeToken()) {
        _ofs << PrintToken(node.GetEdgeToken()) << " " ;
        // Print edge control specifiers
        if (node.GetEdgeDescStr()) {
           _ofs << "[" << node.GetEdgeDescStr() << "]" ;
        }
    }
    if (node.GetTerminalDesc()) node.GetTerminalDesc()->Accept(*this) ;

    if (node.GetCondition()) {
        _ofs << " " << PrintToken(VERI_EDGE_AMPERSAND) << " " ;
        node.GetCondition()->Accept(*this) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriRange, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // []'s are printed in the caller of range
//    _ofs << "[" ;
    if (node.GetLeft()) node.GetLeft()->Accept(*this) ;
    if (node.GetPartSelectToken()) {
        _ofs << PrintToken(node.GetPartSelectToken()) << " " ; // Verilog 2000 and later
    } else {
        _ofs << ":" ; // Verilog 95
    }
    if (node.GetRight()) node.GetRight()->Accept(*this) ;
//    _ofs << "]" ;

    // Print subsequent dimensions also :
    if (node.GetNext()) {
        // but first close previous range and open new one :
        _ofs << "][" ;
        node.GetNext()->Accept(*this) ;
    }
}

void PrettyPrintVisitor::VERI_VISIT(VeriDataType, node)
{
    if (!_bFileGood) return ; // file stream is not good
    if (node.GetType()) _ofs << PrintToken(node.GetType()) << " " ;
    if (node.GetSigning()) _ofs << PrintToken(node.GetSigning()) << " " ;
    if (node.GetDimensions()) {
        _ofs << "[" ;
        node.GetDimensions()->Accept(*this) ;
        _ofs << "] " ;
    }
}

/*-----------------------------------------------------------------*/
//        Visit Methods : Class details in VeriConstVal.h
/*-----------------------------------------------------------------*/

void PrettyPrintVisitor::VERI_VISIT(VeriConst, node)
{
    // Do nothing
}

void PrettyPrintVisitor::VERI_VISIT(VeriIntVal, node)
{
    if (!_bFileGood) return ; // file stream is not good

    _ofs << node.GetNum() ;
}

void PrettyPrintVisitor::VERI_VISIT(VeriRealVal, node)
{
    if (!_bFileGood) return ; // file stream is not good

    char *image = Strings::dtoa(node.GetNum()) ;
    _ofs << image ;
    Strings::free(image) ;
}

#define GET_BIT(S,B) ((S)?(((S)[(B)/8]>>(B)%8)&1):0)

void PrettyPrintVisitor::VERI_VISIT(VeriConstVal, node)
{
    if (!_bFileGood) return ; // file stream is not good

    // Print as a sized bit-array
    // Note : If originally a 'string' constant was used, it will now
    // be written as a bit-pattern.
    if (node.IsString()) {
        char *str = node.Image() ;
        _ofs << str ;
        Strings::free(str) ;
        return ;
    }

    if (node.IsSigned()) { _ofs << node.Size(NULL) << "'sb" ; }
    else                { _ofs << node.Size(NULL) << "'b" ; }

    // Go MSB to LSB. MSB is highest index. 0 the lowest.
    unsigned i = node.Size(NULL) ;
    while (i-- != 0) {
        // Determine the value of this bit
        if (GET_BIT(node.GetXValue(),i))         { _ofs << 'x' ; }
        else if (GET_BIT(node.GetZValue(),i))    { _ofs << 'z' ; }
        else if (GET_BIT(node.GetValue(),i))     { _ofs << '1' ; }
        else                                    { _ofs << '0' ; }
    }
}

/*---------------------------------------------*/
