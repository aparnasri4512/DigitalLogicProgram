#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MaxVars 500
#define MaxGateTypeNameLen 20
#define MaxVar_NameLen 20
#define MaxGateLenline 225

typedef enum { AND, OR, NAND, NOR, XOR, NOT, PASS, DECODER, MULTIPLEXER } GateType;

struct variable {
          char* name;
          int val;
     };


struct gate{
     GateType type;
     int size; //for decoder & multiplexer
     char** inputVariables;
     char** outputVariables;
     int numInputs;
     int numOutputs;
     //multiplexer components
     char** multixSelectors;
     int multixNumSelects;
};

struct gate_Circuit{
     
     char** inputVariables;
     int numInputs;

     char** outputVariables;
     int numOutputs;

     int numGates;
     struct gate *theGates;

     struct variable var_s[MaxVars];
     int numVars;
      
};

//free function - mimic array freeing
void freeCirc(struct gate_Circuit *circuit){
     
     for(int i = 0; i < circuit -> numInputs; i++){
          free(circuit -> inputVariables[i]);
     }
     free(circuit -> inputVariables);

     for(int i = 0; i < circuit -> numOutputs; i++){
          free(circuit -> outputVariables[i]);
     }
     free(circuit -> outputVariables);

     for(int i = 0; i < circuit -> numGates; i++){
          for(int j = 0; j < circuit -> theGates[i].numInputs; j++){

               free(circuit -> theGates[i].inputVariables[j]);

          }
          free(circuit -> theGates[i].inputVariables);

          for(int j = 0; j < circuit -> theGates[i].numOutputs; j++){

               free(circuit -> theGates[i].outputVariables[j]);

          }
          free(circuit -> theGates[i].outputVariables);

          if(circuit -> theGates[i].type == MULTIPLEXER){

               for(int j = 0; j < circuit -> theGates[i].multixNumSelects; j++){

                    free(circuit -> theGates[i].multixSelectors[j]);

               }
               free(circuit -> theGates[i].multixSelectors);
          }

     }

     free(circuit -> theGates);

     for(int i = 0; i < circuit -> numVars; i++){
          
          free(circuit -> var_s[i].name);
     }

     free(circuit);
     
}

void rare(struct gate_Circuit* circuit ){

     freeCirc(circuit);
}


void InVar(struct gate_Circuit* circuit, char* nameVar){

     if((strcmp(nameVar, "1") == 0) || (strcmp(nameVar, "0") == 0)){

          return;
     }

     for(int i = 0; i < circuit -> numVars; i++){

          if(strcmp(circuit -> var_s[i].name, nameVar) == 0){

               return;

          }
     }
     //getting name of circuit
     circuit -> var_s[circuit -> numVars].name = (char*)malloc(sizeof(char)*(strlen(nameVar)+1));

     strcpy(circuit -> var_s[circuit -> numVars].name, nameVar);

     circuit -> var_s[circuit -> numVars].val = 0;

     circuit -> numVars++;
}


void varSet(struct gate_Circuit *circuit, char* nameVar, int val){

     if((strcmp(nameVar, "1") == 0 )||(strcmp(nameVar, "0") == 0) ){

          return;

     }
     
     for(int i = 0; i < circuit -> numVars; i++){

          if(strcmp(circuit -> var_s[i].name, nameVar) == 0){

               circuit -> var_s[i].val = val;

               return;
          }
     }

     //fprintf(stderr, "Error %s setting", nameVar);
}




int getVar(struct gate_Circuit* circuit, char* nameVar){

     if((strcmp(nameVar, "1") == 0 )||(strcmp(nameVar, "0") == 0) ) {
          //returns string name converted to number representation
          return atoi(nameVar);
     }
     

     for(int i = 0; i < circuit -> numVars; i++){
          if(strcmp(circuit -> var_s[i].name, nameVar) == 0){
               
               return circuit -> var_s[i].val;
          }
     }

     //fprintf(stderr, "Error for getting %s var", nameVar);
     
     return 0;
}

struct gate_Circuit* readDiscription(char* file){

     FILE *fp = fopen(file, "r");

     if(fp == NULL){
          //fprintf(stderr, "Reading in error");
          return NULL;
     }
     

     struct gate_Circuit* circuit = malloc(sizeof(struct gate_Circuit));

     circuit -> numVars = 0;

     int numGates;

     char charac;

     numGates = -2;

     while(EOF !=(charac = getc(fp))){

          char lineGate[MaxGateLenline];

          fgets(lineGate, sizeof lineGate, fp);

          if(strlen(lineGate) > 0){

               numGates++;
          }
     }

     circuit -> numGates = numGates;



     rewind(fp);

     char io_gate_type[MaxGateTypeNameLen];
//INPUT READING
     if(fscanf(fp, "%s %d", io_gate_type, &circuit -> numInputs) != 2){


          fclose(fp);

          return NULL;
     }

     //printDebug("Inputs %d\n", circuit -> numInputs);


     circuit -> inputVariables = (char**) malloc(sizeof(char*)*circuit -> numInputs);

     for(int j = 0; j < circuit -> numInputs; j++) {
 
          char varIn[MaxVar_NameLen];

          if (fscanf(fp, "%s", varIn) != 1) {
     

               rare(circuit);

               break;
          }


          circuit-> inputVariables[j]=(char*)malloc(sizeof(char)*(strlen(varIn))+1);

          strcpy(circuit ->inputVariables[j], varIn);
          
          InVar(circuit, circuit -> inputVariables[j]);
     }


//OUTPUT READING
     if (fscanf(fp, "%s %d", io_gate_type, &circuit -> numOutputs) != 2) {

     

          fclose(fp);

          return NULL;

     }
     //print outputs - check outputs


     circuit -> outputVariables =(char**)malloc(sizeof(char*)*circuit -> numOutputs);

     for(int j = 0; j < circuit -> numOutputs; j++) {
 
     char varOut[MaxVar_NameLen];

     if (fscanf(fp, "%s", varOut) != 1) {

         // fprintf(stderr, "Error reading output %s.\n", fp);

          rare(circuit);

          break;
     }

     circuit -> outputVariables[j] = (char*)malloc(sizeof(char)*(strlen(varOut)+1));

     strcpy(circuit -> outputVariables[j], varOut);

     InVar(circuit, circuit -> outputVariables[j]);
     }


     circuit -> theGates = malloc(sizeof(struct gate)*numGates);

     //traversal & add gate-type
     for(int i = 0; i < numGates; i++) {

          char TYPEGate[MaxGateTypeNameLen];

          fscanf(fp, "%s ", TYPEGate);

          if (strcmp(TYPEGate, "AND") == 0) { 

               circuit -> theGates[i].type = AND;

          }else if (strcmp(TYPEGate, "NAND") == 0) {

               circuit -> theGates[i].type = NAND; 

          }else if (strcmp(TYPEGate, "OR") == 0) { 
               circuit -> theGates[i].type = OR; 

          }else if (strcmp(TYPEGate, "NOR") == 0) {
               circuit -> theGates[i].type = NOR;

          }else if (strcmp(TYPEGate, "XOR") == 0) { 
               circuit -> theGates[i].type = XOR; 

          }else if (strcmp(TYPEGate, "NOT") == 0) { 
               circuit -> theGates[i].type = NOT;

          }else if (strcmp(TYPEGate, "PASS") == 0) { 
               circuit -> theGates[i].type = PASS; 

          }else if (strcmp(TYPEGate, "DECODER") == 0) {
               circuit -> theGates[i].type = DECODER;

          }else if (strcmp(TYPEGate, "MULTIPLEXER") == 0) {
               circuit -> theGates[i].type = MULTIPLEXER; 

          }

          //Handling multiplexer & decoder

          int numberSelects;

          int numberIns;

          int numberOutputs;

          //check DECODER & MULTIPLEXER aft enum conversion

          if(circuit -> theGates[i].type == DECODER) {

               fscanf(fp,"%d ", &numberIns);

               numberOutputs = (1 << numberIns);

          }else if(circuit -> theGates[i].type == MULTIPLEXER) {

               fscanf(fp,"%d ", &numberSelects);

               numberIns = (1 << numberSelects);

               numberOutputs = 1;

          }else if(circuit -> theGates[i].type == PASS || circuit -> theGates[i].type == NOT) {

               numberIns = 1;

               numberOutputs = 1;

          }else{

               numberIns = 2;

               numberOutputs = 1;

          }

          circuit -> theGates[i].numInputs = numberIns;

          circuit -> theGates[i].inputVariables = (char**)malloc(sizeof(char*)*numberIns);

          circuit -> theGates[i].numOutputs = numberOutputs;

          circuit -> theGates[i].outputVariables = (char**)malloc(sizeof(char*)*numberOutputs);

          circuit -> theGates[i].size = numberIns;

          for(int j = 0; j < numberIns; j++) { 

               char funcParameters[MaxVar_NameLen];
               
               fscanf(fp, "%s ", funcParameters);

               circuit -> theGates[i].inputVariables[j] = (char*)malloc(sizeof(char)* (strlen(funcParameters)+1));

               strcpy(circuit -> theGates[i].inputVariables[j], funcParameters);

               InVar(circuit , funcParameters);

               //printDebug("%s ~ ", circuit -> theGates[i].inputVariables[j]);
          }

          //MULTIPLEXER
          if(circuit -> theGates[i].type == MULTIPLEXER) {

               circuit -> theGates[i].multixNumSelects = numberSelects;

               circuit -> theGates[i].multixSelectors = (char**)malloc(sizeof(char*)*numberSelects);

               for(int j = 0; j < numberSelects; j++) { 

                    char funcParameters[MaxVar_NameLen];

                    fscanf(fp, "%s ", funcParameters);

                    circuit -> theGates[i].multixSelectors[j] = (char*)malloc(sizeof(char)*(strlen(funcParameters)+1));

                    strcpy(circuit -> theGates[i].multixSelectors[j], funcParameters);

                    //Same as before
                    if((strcmp(funcParameters, "1") != 0) && (strcmp(funcParameters, "0") != 0)) {

                         InVar(circuit , funcParameters);
                     }

               }
          }


          //printDebug(" - ");
          for(int j = 0; j < numberOutputs; j++) { 

               char funcParameters[MaxVar_NameLen];

               fscanf(fp, "%s", funcParameters);

               circuit -> theGates[i].outputVariables[j] = (char*)malloc(sizeof(char)*(strlen(funcParameters)+1));

               strcpy(circuit -> theGates[i].outputVariables[j], funcParameters);

               InVar(circuit, funcParameters);

          }
     }
 
     //error case

     if (circuit == NULL){

          return NULL;
     }
     fclose(fp);

     return circuit;
}


//calculating gates
void Gate_Calculate(struct gate_Circuit *circuit, int index) {

     struct gate *realGate = &circuit -> theGates[index];

     for (int i = 0; i < realGate -> numInputs; ++i) {

        int inputVal = getVar(circuit, realGate->inputVariables[i]);

        varSet(circuit, realGate -> inputVariables[i], inputVal);

    }
     
     //all cases
     switch (realGate -> type) {
          case AND: {
               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int input2 = getVar(circuit, realGate -> inputVariables[1]);

               int output = input1 & input2;

               //printDebug("AND %d %d %d | ", input1, input2, output);

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }

          case OR: {
               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int input2 = getVar(circuit, realGate -> inputVariables[1]);

               int output = input1 | input2;

               //printDebug("OR %d %d %d | ", input1, input2, output);

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }

          case NAND: {

               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int input2 = getVar(circuit, realGate -> inputVariables[1]);

               int output = !(input1 & input2);

               //printDebug("NAND %d %d %d | ", input1, input2, output);

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }

          case NOR: {

               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int input2 = getVar(circuit, realGate -> inputVariables[1]);

               int output = !(input1 | input2);

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }

          case XOR: {
               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int input2 = getVar(circuit, realGate -> inputVariables[1]);

               int output = (input1 ^ input2);

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }
          
          case NOT: {
               
               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int output = !input1;

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }

          case PASS: {
               
               int input1 = getVar(circuit, realGate -> inputVariables[0]);

               int output = input1;

               varSet(circuit, realGate -> outputVariables[0], output);

               break;
          }

          case MULTIPLEXER: {

               int numSelectors = 0;
               for (int i = 0; i < realGate -> multixNumSelects; ++i) {

                    int tempSelector;

                    tempSelector = getVar(circuit, realGate -> multixSelectors[i]);

                    numSelectors = ((numSelectors << 1) | tempSelector);

               }
               int TEMPoutput = getVar(circuit, realGate -> inputVariables[numSelectors]);

               varSet(circuit , realGate -> outputVariables[0], TEMPoutput);

               break;
          }

          case DECODER: {
          //reset index to 0
               int index = 0;

               for (int i = 0; i < realGate -> numInputs; i++) {

                    int TEMPinput = getVar(circuit, realGate ->inputVariables[i]);

                    index = ((index << 1) | TEMPinput);
               }
 
               for (int i = 0; i < realGate -> numOutputs; i++) {

                    int TEMPoutput = (i == index) ? 1 : 0;

                    varSet(circuit, realGate -> outputVariables[i], TEMPoutput);
               }
               break;
          }

     default:
     }
}

/*int** allocateMat(int rows, int cols){
     int** mat = (int**)malloc(rows* sizeof(int*));
     for(int i = 0; i < rows; i++){
          mat[i] = (int*)malloc(cols*sizeof(int));
     }
     return mat;
}

void freeMat(int** mat, int numRows){
     for(int i = 0; i < numRows; i++){
          free(mat[i]);
     }
     free(mat);
}*/

void printTable(struct gate_Circuit* circuit){
     //initialize
     int numberGates = circuit -> numGates;
 
     int numberInputs = circuit -> numInputs;

     int numberOutputs = circuit -> numOutputs;

     //traverse all truth table cases

     for(int i = 0; i <(1<<numberInputs); i++){
          //preset
          int temp = 0;
          //start at end of array
          for(int j = numberInputs-1; j >=0; j--){

               int inputValue = (i&(1<<j)) ? 1:0;

               //printDebug("%s=", circuit -> inputVariables[temp]);

               printf("%d ", inputValue);

               varSet(circuit, circuit -> inputVariables[temp], inputValue);
               //don't forget: increment temp
               temp++;
          }
          printf("| ");

          for(int j = 0; j < numberGates; j++){

               //printDebug("#%d ", j);

               Gate_Calculate(circuit, j);

               //printDebug("\n");
          }

          for(int j = 0; j < numberOutputs; j++){

               printf("%d", getVar(circuit, circuit -> outputVariables[j]));

               if(j < numberOutputs - 1){

                    printf(" ");

               }

          }
          printf("\n");


          fflush(stdout);

     }
}


int main(int argc, char* argv[]){

     //check is file exists
     if(argc !=2){

          return EXIT_FAILURE;

     }

     //call reading function to read file
     struct gate_Circuit* circuit = readDiscription(argv[1]);

     if(circuit == NULL){
          
          return EXIT_FAILURE;
     }

     printTable(circuit);
     freeCirc(circuit); //free circuit
     return EXIT_SUCCESS;
}




   