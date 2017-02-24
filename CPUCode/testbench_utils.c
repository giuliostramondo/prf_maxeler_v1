#include "testbench_utils.h"

extern int N;
extern int M;

void printAGUOutput(int p, int q, Address2d* elements){
    int i;

    for(i=0; i< p*q; i++){
        printf("(%d,%d) ",elements[i].i,elements[i].j);
    }
    printf("\n");
    return;
}

/*
 * Tests if a list of address is within the matrix bound
 * returns 1 if it is whitin bound, 0 otherwise
 */
int checkBoundaries(int p,int q, Address2d* elements){
    int success=1;
    int i;
    for(i=0;i<p*q;i++){
        if (elements[i].i>=N || elements[i].j >=M || elements[i].i<0 || elements[i].j<0){
            success=0;
            return success;
        }
    }
    return success;
}
//Returns the number of parallel access required, with a given shape, to access all the elements of the matrix
int totAccessToScanMatrix(int p, int q, acc_type shape){
    int tot_accesses = 0;
    switch(shape){
        case RECTANGLE:
            //Access required to write in the PRF using RECTANGLE shape
            tot_accesses = (N/p)*(M/q);
            //if N%p!=0 need to add the right edge
            if( N%p != 0){
                tot_accesses+=N/p;

            } 
            //if M%q!=0 need to add the bottom edge
            if (M%q!=0){
                tot_accesses+=M/q;
            }
            //if N%p!=0 and M%q!=0 add bottom right elements
            if( N%p !=0 && M%q !=0){
                tot_accesses+=1;
            }
            break;

        case ROW:
            //Access required to write in the PRF using ROW shape
            tot_accesses = N*(M/(p*q));
            //if M%(p*q) need to add the right edge
            if( M%(p*q) != 0){
                tot_accesses+=N;
            } 
            break;
        case COLUMN:
            //Access required to write in the PRF using COLUMN shape
            tot_accesses = (N/(p*q))*M;
            //if N%(p*q)!=0 need to add the bottom edge
            if( N%(p*q) != 0){
                tot_accesses+=M;
            } 
            break;
        case MAIN_DIAG:
            return totAccessToScanMatrix(p*q,p*q,RECTANGLE);
        case SECONDARY_DIAG:
            return totAccessToScanMatrix(p*q,p*q,RECTANGLE);
        case TRANSP_RECTANGLE:
           return totAccessToScanMatrix(q,p,RECTANGLE);
        default:
            return 0;
    }       
    return tot_accesses;
}

int32_t* all_accesses(int p, int q, acc_type shape, int N, int M, int *el_counter){
   // input_data contains p*q integers of input_data;
   // i,j coordinates of the block access
   // int of access type ( RECTANGLE ->0 , ROW -> 1, COL->2, MAIN_DIAG -> 3, SECONDARY_DIAG ->4, TRANS_RECTANGLE -> 5
   // write enable signal 1 or 0
   // repeated size time (min 4  probably )
    int32_t *input_data;
    int i,j,tot_accesses,k,counter=0;

    int input_data_size=0;

    for(i=0;i<N;i++){
        for(j=0;j<M;j++){

            Address2d* elements_to_access = AGU(i,j,p,q,shape);
            if(checkBoundaries(p,q,elements_to_access)){
                        input_data_size++;
            }
        }
    }
    input_data =(int32_t*) malloc((sizeof(int32_t)*((p*q)+4))*input_data_size);

    for(i=0;i<N;i++){
        for(j=0;j<M;j++){
            Address2d* elements_to_access = AGU(i,j,p,q,shape);
            if(checkBoundaries(p,q,elements_to_access)){
                    //Fill the data fields to 0, this part will be ignored
                    //by the PRF because a read is being performed
                    for(k=0;k<p*q;k++){
                        input_data[counter*(p*q+4)+k]=0;
                    }

                    input_data[counter*(p*q+4)+p*q] = i;
                    input_data[counter*(p*q+4)+p*q+1] = j;
                    input_data[counter*(p*q+4)+p*q+2] = shape;
                    input_data[counter*(p*q+4)+p*q+3] = 0; // read 
                    counter++;
            }
        }
    }
    *el_counter = counter;
    return input_data;
}

int32_t* fillPRF(int p, int q, scheme s, int **A_test, int *el_counter){
   // input_data contains p*q integers of input_data;
   // i,j coordinates of the block access
   // int of access type ( RECTANGLE ->0 , ROW -> 1, COL->2, MAIN_DIAG -> 3, SECONDARY_DIAG ->4, TRANS_RECTANGLE -> 5
   // write enable signal 1 or 0
   // repeated size time (min 4  probably )
    int32_t *input_data;
    int i,j,tot_accesses,k,counter=0;

    switch(s){
        case RECT_ROW:
            tot_accesses = totAccessToScanMatrix(p,q,RECTANGLE);
            input_data = (int32_t*) malloc((sizeof(int32_t)*p*q+4*sizeof(int32_t))*tot_accesses);
            for(i=0;i<N;i+=p){
                for(j=0;j<M;j+=q){
                    Address2d *elements_to_write = AGU(i,j,p,q,RECTANGLE);
                    for(k=0;k<p*q;k++){
                        input_data[counter*(p*q+4)+k]=A_test[elements_to_write[k].i][elements_to_write[k].j];
                    }
                    input_data[counter*(p*q+4)+p*q] = i;
                    input_data[counter*(p*q+4)+p*q+1] = j;
                    input_data[counter*(p*q+4)+p*q+2] = RECTANGLE;
                    input_data[counter*(p*q+4)+p*q+3] = 0xFFFF; // write 
                    counter++;
                }
            }
            //Write the right edge
            if ( N%p != 0 ){
                j = M-q;
                for(i=0; i<N ; i+=p ){
                    Address2d *elements_to_write = AGU(i,j,p,q,RECTANGLE);
                    for(k=0;k<p*q;k++){
                        input_data[counter*(p*q+4)+k]=A_test[elements_to_write[k].i][elements_to_write[k].j];
                    }
                    input_data[counter*(p*q+4)+p*q] = i;
                    input_data[counter*(p*q+4)+p*q+1] = j;
                    input_data[counter*(p*q+4)+p*q+2] = RECTANGLE;
                    input_data[counter*(p*q+4)+p*q+3] = 1; // write 
                    counter++;
                }
            }
            //Write the bottom edge
            if ( M%q != 0 ){
                i = N-p;
                for(j=0; j<M ; j+=q){
                    Address2d *elements_to_write = AGU(i,j,p,q,RECTANGLE);
                    for(k=0;k<p*q;k++){
                        input_data[counter*(p*q+4)+k]=A_test[elements_to_write[k].i][elements_to_write[k].j];
                    }
                    input_data[counter*(p*q+4)+p*q] = i;
                    input_data[counter*(p*q+4)+p*q+1] = j;
                    input_data[counter*(p*q+4)+p*q+2] = RECTANGLE;
                    input_data[counter*(p*q+4)+p*q+3] = 1; // write 
                    counter++;
                
                }
            }
            //Write bottom right edge
            if(N%p != 0 && M%q!=0){
                i= N-p;
                j= M-q;
                Address2d *elements_to_write = AGU(i,j,p,q,RECTANGLE);
                for(k=0;k<p*q;k++){
                    input_data[counter*(p*q+4)+k]=A_test[elements_to_write[k].i][elements_to_write[k].j];
                }
                input_data[counter*(p*q+4)+p*q] = i;
                input_data[counter*(p*q+4)+p*q+1] = j;
                input_data[counter*(p*q+4)+p*q+2] = RECTANGLE;
                input_data[counter*(p*q+4)+p*q+3] = 1; // write 
                counter++;
            
            }
            break;

        case RECT_COL:
                    input_data = fillPRF(p, q, RECT_ROW, A_test, el_counter);
                    break;

        case ROW_COL:
                    tot_accesses = totAccessToScanMatrix(p,q,ROW);
                    input_data = (int32_t*) malloc((sizeof(int32_t)*p*q+4*sizeof(int32_t))*tot_accesses);
                    for(i=0; i<N ; i++){
                        for( j=0; j<M;j+=p*q){
                            Address2d *elements_to_write = AGU(i,j,p,q,ROW);
                            for(k=0;k<p*q;k++){
                                input_data[counter*(p*q+4)+k]=A_test[elements_to_write[k].i][elements_to_write[k].j];
                            }
                            input_data[counter*(p*q+4)+p*q] = i;
                            input_data[counter*(p*q+4)+p*q+1] = j;
                            input_data[counter*(p*q+4)+p*q+2] = ROW;
                            input_data[counter*(p*q+4)+p*q+3] = 1; // write 
                            counter++;
                        }
                    }
                    //Write right edge
                    if(j%(p*q) != 0 ){
                        j=M-(p*q);
                        for(i=0;i<N;i++){
                            Address2d *elements_to_write = AGU(i,j,p,q,ROW);
                            for(k=0;k<p*q;k++){
                                input_data[counter*(p*q+4)+k]=A_test[elements_to_write[k].i][elements_to_write[k].j];
                            }
                            input_data[counter*(p*q+4)+p*q] = i;
                            input_data[counter*(p*q+4)+p*q+1] = j;
                            input_data[counter*(p*q+4)+p*q+2] = ROW;
                            input_data[counter*(p*q+4)+p*q+3] = 1; // write 
                            counter++;

                        }
                    }
                    break;

        case RECT_TRECT:
                    input_data = fillPRF(p,q,RECT_ROW, A_test, el_counter);
                    break;
    }
    *el_counter=counter;
    return input_data;
}

int32_t* generate_expected_output(int p, int q, int** A_test, int32_t* input_data, int size){
    int i,j;
    int32_t* expected_output = (int32_t*) malloc(sizeof(int32_t)*p*q*size);
    for(i=0;i<size;i++){
        int i_coordinate = input_data[i*(p*q+4)+p*q];
        int j_coordinate = input_data[i*(p*q+4)+p*q+1];
        acc_type shape = input_data[i*(p*q+4)+p*q+2];
        Address2d *elements_to_access = AGU(i_coordinate,j_coordinate,p,q,shape);
        for (j=0;j<p*q;j++){
            Address2d current = elements_to_access[j];
            int element = A_test[current.i][current.j];
            expected_output[i*(p*q)+j]=element;
        }
    }
    return expected_output;
}
