using namespace std ;
#include <bits/stdc++.h>
#include <fstream>
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
#include "VeriRuntimeFlags.h"
#include "VeriMisc.h"
#ifdef VERIFIC_NAMESPACE
using namespace Verific ;
#endif
 //static void TraverseVerilog(const VeriModule *top) ;

 int main(int argc, const char **argv)
 {
     char *file_name = (char *)"alu.v" ;
     char *top_name = (char *)"mAlu" ;
     char *work_lib = (char *)"work" ;
     unsigned vlog_mode = 1 ;
     veri_file veri_reader ;
     if (!veri_reader.Analyze(file_name, vlog_mode, work_lib)) return 1 ;
     if (veri_file::GetModule(top_name)) {
         if (!veri_file::ElaborateStatic(top_name)) return 0 ;
         VeriModule *top_module = veri_file::GetModule(top_name) ;
         if (!top_module) {
             return 0 ;
         }





         string sparam="";
         string spardec = "",sparini = "";
         //module->AnalyzeFull();
         //top_module->PrettyPrint(cout,0);
         Array *para = top_module->GetParameters();
         for(int i=0;i<para->Size();i++){
           VeriIdDef *param = (VeriIdDef *)para->At(i);
           string val = param->GetInitialValue()->GetPrettyPrintedString();
           if(param->IsArray()){
             //cout<<param->GetMsbOfRange()<<endl;
             string rang;
             int app = 0;
             for(int j=0;j<val.length();j++){
               char ch = val[j];
               if(ch != '\'')
               rang = rang + ch;
               else {
                 app = j;
                 break;
               }
             }
             spardec = spardec + "var " + param->Name() + " : " + "bv" + rang+ " ;\n";
             sparini = sparini + "\t" + param->Name() + " = " + val.substr(app+2) + "bv" + rang + " ;\n";
           }//->GetPrettyPrintedString()<<endl;
           else{
             spardec = spardec + "var " + param->Name() + " : " + "integer ;\n";
             sparini = sparini + "\t" + param->Name() + " = " + val + " ;\n";
           }
         }
         sparam = spardec + "init {\n" + sparini + "}\n";
         cout<<sparam;





         Array *por = top_module->GetPorts();
         string spor = "";
         //top_module->PrettyPrintPortConnects(cout,0);
         for(int i=0;i<por->Size();i++){
           VeriIdDef *po = (VeriIdDef *)por->At(i);
           if(po->IsInput()){
             if(po->IsArray()){
               string stri = to_string(po->GetMsbOfRange()+1);
               spor = spor + "input " + po->GetPrettyPrintedString() + " : " + "bv"+stri + " ;\n";
               }
             else{
               spor = spor + "input " + po->GetPrettyPrintedString() + " : " + "bv1 ;\n";
             }
           }
           else {
             if(po->IsArray()){
               spor = spor + "output " + po->GetPrettyPrintedString() + " : " + "bv"+to_string(po->GetMsbOfRange()+1) + " ;\n";
               }
             else{
               spor = spor + "output " + po->GetPrettyPrintedString() + " : " + "bv1 ;\n";
             }
           }

         }
         cout<<spor;







         Array *items = top_module->GetItems();
         string sreg = "";
         for(int i=0;i<items->Size();i++){
           VeriModuleItem *item =(VeriModuleItem *) items->At(i);
           if(item->IsRegDecl()){
             //cout<<item->GetPrettyPrintedString();
             if(item->GetDataType()->GetDimensions()){
               string s = item->GetPrettyPrintedString();
               int flag = 0;
               string svar = "";
               for(int j=0;j<s.length();j++){
                 char c = s[j];
                 if(c==']'){
                 flag = 1;
                 continue;
               }

                 if(flag == 1 && c != ' ' && c != ',' && c!=';'){
                   svar = svar + c;
                 }
                 else if(c == ',' || c == ';'){
                   sreg = sreg + "var " + svar + " : " + "bv" + to_string(item->GetDataType()->GetDimensions()->GetMsbOfRange() + 1) + " ;\n";
                   svar = "";
                 }
               }
              }
              else {
                string s = item->GetPrettyPrintedString();
                int flag = 0;
                string svar = "";
                for(int j=0;j<s.length();j++){
                  char c = s[j];
                  if(c==' '){
                    flag = 1;
                    continue;
                  }                  
                  if(flag == 1 && c != ' ' && c != ',' && c!=';'){
                    svar = svar + c;
                  }
                  else if(c == ',' || c == ';'){
                    sreg = sreg + "var " + svar + " : " + "bv" + "1" + " ;\n";
                    svar = "";
                  }
                }
              }
           }
         }
         cout<<sreg;
         /*Array *re = (Array *)top_module->GetNetAssigns();
         Map *ma = (Map *)top_module->GetAttributes();
         cout<<"sdmvsfdf;gkd;fl"<<endl;
         cout<<top_module<<endl;
         if(!re){
           cout<<"Hello f"<<re<<endl;
         }
         if(!ma){
           cout<<"Hello f"<<ma<<endl;
         }
         cout<<re->Size();*/

         /*for(int i=0;i<re->Size();i++){
           VeriNetRegAssign *regis = (VeriNetRegAssign *)re->At(i);
           cout<<regis->GetPrettyPrintedString();
         }*/
         /*Array *items = top_module->GetItems();
         for(int i=0;i<items->Size();i++){
           VeriModuleItem *item = (VeriModuleItem *)items->At(i);
           /*string s = item->GetPrettyPrintedString();
            vector <string> vs;
            string vss = "";
            int bloc = 0;
            for(int j = 0;j<s.length();j++){
              char ch = s[j];
              if ((ch == " " || ch == ";" || ch == ",") && bloc == 0){

              }
              else if(ch == "[")
              bloc = 1;
              else if(ch == "]")
              bloc = 0;
              else if(ch == "(" || ch == ")")
              continue;
              else if
            }
            if(item->IsRegDecl()){
              cout<<item->GetPrettyPrintedString()<<endl;
            }
         }*/










         /*
           string s = item->GetPrettyPrintedString();
           //cout<<s;
           string s1 = "";
           int fs = 0;
           vector <string> v;
           for(int i=0;i<s.length();i++){
             char c = s[i];
             if(c != ' ')
             s1 = s1 + c;
             else{
               fs = i;
             break;
           }
           }
           cout<<s1<<endl;
           if( s1 == "wire"){

           }
           else if ( s1 == "input"){

           }
           else if(s1 == "output"){

           }
           else if(s1 == "parameter"){

           }
           else if(s1 == "reg"){

           }
           else if(s1 == )
         }*/

         //cout<<param->Name()<<endl;
         top_module->Info("Start hierarchy traversal here at Verilog top level module '%s'", top_module->Name()) ;
        // TraverseVerilog(top_module) ; // Traverse top level module and the hierarchy under it
     }

     return 0 ; // all good
}
