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
#include <VeriModuleItem.h>
#include <VeriId.h>
#ifndef _VERIFIC_VERI_PRETTYPRINT_VISITOR_H_
#define _VERIFIC_VERI_PRETTYPRINT_VISITOR_H_

#include "VeriVisitor.h"    // Visitor base class definition

#include <fstream>

#ifdef VERIFIC_NAMESPACE
namespace Verific { // start definitions in verific namespace
#endif

/* -------------------------------------------------------------------------- */

class PrettyPrintVisitor : public VeriVisitor
{
public:
    PrettyPrintVisitor(char *pFileName);
    virtual ~PrettyPrintVisitor();

/* ================================================================= */
/*                         VISIT METHODS                             */
/* ================================================================= */

    // The following class definitions can be found in VeriTreeNode.h
    virtual void VERI_VISIT(VeriTreeNode, node);

    // The following class definitions can be found in VeriModule.h
    virtual void VERI_VISIT(VeriModule, node);
    virtual void VERI_VISIT(VeriPrimitive, node);

    // The following class definitions can be found in VeriExpression.h
    virtual void VERI_VISIT(VeriExpression, node);
    virtual void VERI_VISIT(VeriIdRef, node);
    virtual void VERI_VISIT(VeriIndexedId, node);
    virtual void VERI_VISIT(VeriSelectedName, node);
    virtual void VERI_VISIT(VeriIndexedMemoryId, node);
    virtual void VERI_VISIT(VeriConcat, node);
    virtual void VERI_VISIT(VeriMultiConcat, node);
    virtual void VERI_VISIT(VeriFunctionCall, node);
    virtual void VERI_VISIT(VeriSystemFunctionCall, node);
    virtual void VERI_VISIT(VeriMinTypMaxExpr, node);
    virtual void VERI_VISIT(VeriUnaryOperator, node);
    virtual void VERI_VISIT(VeriBinaryOperator, node);
    virtual void VERI_VISIT(VeriQuestionColon, node);
    virtual void VERI_VISIT(VeriEventExpression, node);
    virtual void VERI_VISIT(VeriPortConnect, node);
    virtual void VERI_VISIT(VeriPortOpen, node);
    virtual void VERI_VISIT(VeriAnsiPortDecl, node);
    virtual void VERI_VISIT(VeriTimingCheckEvent, node);
    virtual void VERI_VISIT(VeriDataType, node);

    // The following class definitions can be found in VeriId.h
    virtual void VERI_VISIT(VeriIdDef, node);
    virtual void VERI_VISIT(VeriVariable, node);
    virtual void VERI_VISIT(VeriInstId, node);
    virtual void VERI_VISIT(VeriModuleId, node);
    virtual void VERI_VISIT(VeriUdpId, node);
    virtual void VERI_VISIT(VeriTaskId, node);
    virtual void VERI_VISIT(VeriFunctionId, node);
    virtual void VERI_VISIT(VeriGenVarId, node);
    virtual void VERI_VISIT(VeriParamId, node);
    virtual void VERI_VISIT(VeriBlockId, node);

    // The following class definitions can be found in VeriMisc.h
    virtual void VERI_VISIT(VeriRange, node);
    virtual void VERI_VISIT(VeriStrength, node);
    virtual void VERI_VISIT(VeriNetRegAssign, node);
    virtual void VERI_VISIT(VeriCaseItem, node);
    virtual void VERI_VISIT(VeriGenerateCaseItem, node);
    virtual void VERI_VISIT(VeriPath, node);
    virtual void VERI_VISIT(VeriDelayOrEventControl, node);

    // The following class definitions can be found in VeriModuleItem.h
    virtual void VERI_VISIT(VeriModuleItem, node);
    virtual void VERI_VISIT(VeriDataDecl, node);
    virtual void VERI_VISIT(VeriNetDecl, node);
    virtual void VERI_VISIT(VeriFunctionDecl, node);
    virtual void VERI_VISIT(VeriTaskDecl, node);
    virtual void VERI_VISIT(VeriDefParam, node);
    virtual void VERI_VISIT(VeriContinuousAssign, node);
    virtual void VERI_VISIT(VeriGateInstantiation, node);
    virtual void VERI_VISIT(VeriModuleInstantiation, node);
    virtual void VERI_VISIT(VeriSpecifyBlock, node);
    virtual void VERI_VISIT(VeriPathDecl, node);
    virtual void VERI_VISIT(VeriSystemTimingCheck, node);
    virtual void VERI_VISIT(VeriInitialConstruct, node);
    virtual void VERI_VISIT(VeriAlwaysConstruct, node);
    virtual void VERI_VISIT(VeriGenerateConstruct, node);
    virtual void VERI_VISIT(VeriGenerateConditional, node);
    virtual void VERI_VISIT(VeriGenerateCase, node);
    virtual void VERI_VISIT(VeriGenerateFor, node);
    virtual void VERI_VISIT(VeriGenerateBlock, node);
    virtual void VERI_VISIT(VeriTable, node);

    // The following class definitions can be found in VeriStatement.h
    virtual void VERI_VISIT(VeriStatement, node);
    virtual void VERI_VISIT(VeriBlockingAssign, node);
    virtual void VERI_VISIT(VeriNonBlockingAssign, node);
    virtual void VERI_VISIT(VeriGenVarAssign, node);
    virtual void VERI_VISIT(VeriAssign, node);
    virtual void VERI_VISIT(VeriDeAssign, node);
    virtual void VERI_VISIT(VeriForce, node);
    virtual void VERI_VISIT(VeriRelease, node);
    virtual void VERI_VISIT(VeriTaskEnable, node);
    virtual void VERI_VISIT(VeriSystemTaskEnable, node);
    virtual void VERI_VISIT(VeriDelayControlStatement, node);
    virtual void VERI_VISIT(VeriEventControlStatement, node);
    virtual void VERI_VISIT(VeriConditionalStatement, node);
    virtual void VERI_VISIT(VeriCaseStatement, node);
    virtual void VERI_VISIT(VeriForever, node);
    virtual void VERI_VISIT(VeriRepeat, node);
    virtual void VERI_VISIT(VeriWhile, node);
    virtual void VERI_VISIT(VeriFor, node);
    virtual void VERI_VISIT(VeriWait, node);
    virtual void VERI_VISIT(VeriDisable, node);
    virtual void VERI_VISIT(VeriEventTrigger, node);
    virtual void VERI_VISIT(VeriSeqBlock, node);
    virtual void VERI_VISIT(VeriParBlock, node);

    // The following class definitions can be found in VeriConstVal.h
    virtual void VERI_VISIT(VeriConst, node);
    virtual void VERI_VISIT(VeriConstVal, node);
    virtual void VERI_VISIT(VeriIntVal, node);
    virtual void VERI_VISIT(VeriRealVal, node);

    // This is the only special case we need to take care of
    void PrettyPrintWOSemi(VeriBlockingAssign &node);
    void PrettyPrintWOSemi(VeriDataDecl &node);

public:
    // Did the file open for output correctly?
    bool IsFileGood() const { return _bFileGood; }

private:
    std::ofstream    _ofs;         // Output file stream
    bool            _bFileGood;    // States whether the file was opened correctly
    unsigned        _nLevel;       // Indentation level - used for output blank spaces

    // _nLevel modifiers
    void IncTabLevel(unsigned nIncVal)     { _nLevel += nIncVal ; }    // Increase indentation level
    void DecTabLevel(unsigned nDecVal)     { _nLevel -= nDecVal ; }    // Decrease indentation level

    /* ================================================================= */
    /*                    OUTPUT UTILITY METHODS                         */
    /* ================================================================= */

    // Print tab indentation based on level
    static const char* PrintLevel(unsigned level);

    // Determine if the string represents a verilog escaped identifier
    static unsigned IsEscapedIdentifier(const char *str);

    // Print indentifier correctly (ie. hierarchical and/or escaped)
    static void PrintIdentifier(std::ostream &f, const char *str);

    // Print token characters (definition below)
    static const char* PrintToken(unsigned veri_token);

    // Prevent the compiler from implementing the following
    PrettyPrintVisitor(const PrettyPrintVisitor &node);
    PrettyPrintVisitor& operator=(const PrettyPrintVisitor &rhs);
} ;

/* -------------------------------------------------------------------------- */

#ifdef VERIFIC_NAMESPACE
} // end definitions in verific namespace
#endif

#endif // #ifndef _VERIFIC_VERI_PRETTYPRINT_VISITOR_H_
