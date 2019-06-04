using namespace std ;
#include <bits/stdc++.h>
#include "Array.h"
#include "Map.h"
#include "Set.h"
#include "Message.h"
#include "Strings.h"
#include "veri_file.h"
#include "VeriModule.h"
#include "VeriExpression.h"
#include "VeriId.h"
#include "VeriScope.h"
#ifdef VERIFIC_NAMESPACE
using namespace Verific ;
#endif
static void TraverseVerilog(const VeriModule *top) ;
int main(int argc, const char **argv)
 {
    char *file_name = (char *)"r4000.v" ;
    char *top_name = (char *)"mR4000" ;
    char *work_lib = (char *)"work" ;
    unsigned vlog_mode = 1 ;
    veri_file veri_reader ;
    if (!veri_reader.Analyze(file_name, vlog_mode, work_lib)) 
        return 1 ; 
     if (veri_file::GetModule(top_name)) {
         if (!veri_file::ElaborateStatic(top_name)) return 0 ;  
         VeriModule *top_module = veri_file::GetModule(top_name) ;
         if (!top_module) {
             return 0 ; 
         } 
         top_module->Info("Start hierarchy traversal here at Verilog top level module '%s'", top_module->Name()) ;
         TraverseVerilog(top_module) ; 
     }
 
     return 0 ; 
 }
 
static void
TraverseVerilog(const VeriModule *module)
{   
    if (!module) return ;
    VeriScope *scope = module->GetScope();
    Map *ids = scope ? scope->DeclArea() : 0 ;
     MapIter mi ;
     VeriIdDef *id ;
     FOREACH_MAP_ITEM(ids, mi, 0, &id) { 
         if (!id || !id->IsInst()) continue ;
         VeriModuleInstantiation *mod_inst = id->GetModuleInstance() ;
         VeriModule *mod = mod_inst ? mod_inst->GetInstantiatedModule() : 0 ;
         Message::PrintLine("Processing instance : ", id->Name()) ;
         if (mod) { 
             Message::PrintLine("instantiated module : ", mod->Name()) ;
             Message::PrintLine("\n") ;
             TraverseVerilog(mod) ; 
         }
     }
 }