/*
    Copyright 2016 Giulio Stramondo

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "prf.h"
#include "utility.h"
#include "testbench_utils.h"

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"

int N;
int M;


int main(int argc, char* argv[])
{
    int i,j,k,w;
    Options parameters = parseArguments(argc,argv);
    if(parameters.error)
	return 0;
    int p = parameters.p;
    int q = parameters.q;
    N = parameters.N;
    M = parameters.M;
    scheme s = parameters.s;
    //int A_test[N][M];
    
    int **A_test = (int**)malloc(sizeof(int*)*N);

    for(i=0;i<N;i++){
        A_test[i]= (int*)malloc(sizeof(int)*M);
    }
    for(i=0;i<N;i++){
        for(j=0;j<M;j++){
            A_test[i][j]= i*M+j;
        }
    }
    int32_t* input_data;
    int32_t* output_data;
    int counter=0;
	char* scheme_name = schemeStringFromMappingScheme(s);
        printf("Testing %s scheme ...\n",scheme_name);
        printf("Filling PRF...\n");
        input_data = fillPRF( p, q, RECT_ROW,A_test,  &counter);
    	output_data = (int32_t*) malloc(sizeof(int32_t)*p*q*counter);
        //Add here first call to the PRF on maxeler board
    	prf_complete_v1( counter, input_data,output_data);
        //PRF_Complete( counter, input_data,output_data);
        t_list* available_acc_type = getAvaliableAccessType(s);
        while(available_acc_type!=NULL){
            acc_type* curr_shape = (acc_type*) available_acc_type->data;
            char* shape_name = accessStringFromAccessType(*curr_shape); 
            printf("Testing %s shape...\n",shape_name);
            input_data = all_accesses(p,q,*curr_shape,N,M,&counter); 
            int32_t* expected_output = generate_expected_output(p,q,A_test,input_data,counter);
            int32_t* output_data = (int32_t*) malloc(sizeof(int32_t)*p*q*counter);
	    //Send the input_data to the PRF and collect outputs
           // PRF_Complete( counter, input_data,output_data);
            prf_complete_v1( counter, input_data,output_data);

            int error = 0;
            //Check equivalence   
            for (j=0;j<counter;j++){
                for(k=0;k<p*q;k++){
                    if(output_data[j*(p*q)+k] != expected_output[j*(p*q)+k]){
                        error=1;
                    }
                }
                if(error){
                    //Print error
                    printf("\033[31m[Error]\033[0m\t output %d\n",j);
                    printf("Input: ");
                     for(w=0;w<(p*q)+4;w++){
                        printf("%d ",input_data[j*((p*q)+4)+w]);
                    }
                    printf("\n");
                    printf("Expected Output: ");
		    for(w=0;w<p*q;w++){
                        printf("%d ",expected_output[j*(p*q)+w]);
                    }
                    printf("\n");
                    printf("Output: ");
                    for(w=0;w<p*q;w++){
                        printf("%d ",output_data[j*(p*q)+w]);
                    }
                    printf("\n");
                    error=0;
                }else{
			printf("\033[32m[Correct]\033[0m\t output %d\n",j);
			printf("Input: ");
                     	for(w=0;w<(p*q)+4;w++){
                        	printf("%d ",input_data[j*((p*q)+4)+w]);
                    	}
                    	printf("\n");
                    	printf("Output: ");
                    	for(w=0;w<p*q;w++){
                        	printf("%d ",output_data[j*(p*q)+w]);
                    	}
                    	printf("\n");
 
		}
            }
            available_acc_type = available_acc_type->next;
		
        } 
        
    
    
    return 0;    
}
