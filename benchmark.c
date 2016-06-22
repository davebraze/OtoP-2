#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <mikenet/simulator.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "model.h"

// parameters for phonemes
#define PHO_FEATURES 11
#define PHO_NUMBER 35
#define PHO_OUT 55
#define phoF "./phonemes.txt"
#define exTrF "./training_examples.txt"
#define exTeF "./training_examples.txt"	// currently the same as exTrF;
typedef struct
{ char name;	// name of phoneme;
  Real vec[PHO_FEATURES];	// features of phoneme;
} Phoneme;
Phoneme *phon=NULL;

// parameters for recording timepoints, total iteration, and seed;
#define SEED (long)(time(NULL))
#define REP 1000
#define ITER 5000
#define V_METHOD 0
#define V_THRES 0.5

#define Rand0_1 rand()/(RAND_MAX+1.0)
#define _FileLen 300


// functions;
void load_phoneme(char *PhoFileName)
{ // initialize phon by reading from PhoFileName;
	assert(PhoFileName!=NULL);
	int curphon, curvec;
	FILE *f=NULL;
	char line[255], *p=NULL;
	// initialize PhoDic;
	phon=malloc(PHO_NUMBER*sizeof(Phoneme)); assert(phon!=NULL);
	// read from PhoFileName;
	if((f=fopen(PhoFileName,"r"))==NULL){ printf("Can't open %s\n", PhoFileName); exit(1); }
	curphon=0;
	fgets(line, 255, f);
	while(!feof(f))
    	{ phon[curphon].name=strtok(line," ")[0];
		  curvec=0;
		  while(p=strtok(NULL, " "))
			{ phon[curphon].vec[curvec]=atof(p); 
			  curvec++;
			}
		  curphon++;
		  fgets(line,255,f);
		}
	fclose(f);
	/*
	// testing: print PhoDic
	int i, j;
	for(i=0;i<PHO_NUMBER;i++)
		{ printf("Phoneme %c: ", phon[i].name);
		  for(j=0;j<PHO_FEATURES;j++)
	  		printf("%2.1f ", phon[i].vec[j]);
	     	  printf("\n");
		}
	*/
}


int count_connections(Net *net)
{ // calculate number of connections in the network;
	assert(net!=NULL);
  	int i, j, k, count=0;
  	for(i=0;i<net->numConnections;i++)
    	count += net->connections[i]->from->numUnits * 
    net->connections[i]->to->numUnits;
  	return count;
}


float euclid_dist(Real *x1, Real *x2)
{ // calculate euclidean distance between two vectors x1 and x2;
	assert(x1!=NULL); assert(x2!=NULL);
	int i;
  	float dist=0.0;
  	for(i=0;i<PHO_FEATURES;i++)
    	dist+=(x1[i]-x2[i])*(x1[i]-x2[i]);
  	return dist;
}


int vect_check(Real *x1, Real *x2, double v_thres)
{ // check vector's bit-based threshold between two vectors x1 and x2;
	assert(x1!=NULL); assert(x2!=NULL);
	int i, inThres=1;
	for(i=0;i<PHO_FEATURES;i++)
		{ if(abs(x1[i]-x2[i])>=v_thres) { inThres=0; break;}
		}
	return inThres;
}


void FindPhoneme(Real *vect, Real *trans, int v_method, double v_thres)
{ // find phoneme matching vect; 
	assert(vect!=NULL); assert(trans!=NULL); 
	int i, j, ind, MaxDist, numInThres, *InThresSet=NULL, curind;
	
	switch(v_method)
		{ case 0: // using phoneme with smallest Euclidean distance to vect as trans;
		  		ind=-1; MaxDist=1e6;
		  		for(i=0;i<PHO_NUMBER;i++)
					{ if(euclid_dist(vect,phon[i].vec)<=MaxDist) { ind=i; MaxDist=euclid_dist(vect,phon[i].vec); }
					}
		  		assert(ind!=-1);
		  		for(i=0;i<PHO_FEATURES;i++)
					trans[i]=phon[ind].vec[i];
				break;
		
		  case 1: // using phoneme with 0.5 threshold to set trans;
		  		numInThres=0;
		  		for(i=0;i<PHO_NUMBER;i++)
					{ if(vect_check(vect,phon[i].vec,v_thres)) numInThres++;
					}
		  		if(numInThres==0)
					{ // no such phoneme;
		  	  		  for(i=0;i<PHO_FEATURES;i++)
						trans[i]=-2.0;
					}
		  		else if(numInThres==1)
					{ // there is only one phoneme that matches this requirement;
			  		  for(i=0;i<PHO_NUMBER;i++)
						{ if(vect_check(vect,phon[i].vec,v_thres))
							{ for(j=0;j<PHO_FEATURES;j++)
								trans[j]=phon[i].vec[j];
							  break;
							}
						}
					}
		  		else
					{ // there are more than one phoneme that match this requirement, randomly select one!
			  		  InThresSet=malloc(numInThres*sizeof(int)); assert(InThresSet!=NULL);
			  		  curind=0;
			  		  for(i=0;i<PHO_NUMBER;i++)
						{ if(vect_check(vect,phon[i].vec,v_thres)) { InThresSet[curind]=i; curind++; }
						}
			  		  ind=Rand0_1*numInThres;
			  		  for(i=0;i<PHO_FEATURES;i++)
						trans[i]=phon[InThresSet[ind]].vec[i];
			  		  free(InThresSet); InThresSet=NULL;
					}
				break;
		 default: break;		
		}
}


Real calaccu(Real *out, Real *target, int v_method, double v_thres)
{ // calculate accuracy by comparing out with target;
	assert(out!=NULL); assert(target!=NULL); 
	int i, j, same, NoAccu, NoPho;
	Real *vect=NULL, *trans=NULL, *transout=NULL;	// vect is each segment of out, transout is translated out based on accuMethod;

	transout=malloc(PHO_OUT*sizeof(Real)); assert(transout!=NULL);
	// translate
	for(i=0;i<PHO_OUT;i+=PHO_FEATURES)
		{ vect=malloc(PHO_FEATURES*sizeof(Real)); assert(vect!=NULL);
		  trans=malloc(PHO_FEATURES*sizeof(Real)); assert(trans!=NULL);

		  for(j=0;j<PHO_FEATURES;j++)
			vect[j]=out[i+j];
		  FindPhoneme(vect, trans, v_method, v_thres);
		  for(j=0;j<PHO_FEATURES;j++)
			transout[i+j]=trans[j];
		
		  free(vect); vect=NULL;
		  free(trans); trans=NULL;
		}
	
	// check correct translation
	NoAccu=0; NoPho=1;
	for(i=0;i<PHO_OUT;i+=PHO_FEATURES)
		{ same=1;
		  for(j=0;j<PHO_FEATURES;j++)
			{ if(transout[i+j]!=target[i+j]) { same=0; break; }
			}
		  if(same==1) NoAccu++;
		  NoPho++;
		}
	return NoAccu/(float)(NoPho);	
}


Real getAccu(Net *net, ExampleSet *examples, int iter, int v_method, double v_thres, FILE *f)
{ // calculate accuracy of the network;
	assert(net!=NULL); assert(examples!=NULL); assert(f!=NULL);
	int i, j;
	Example *ex=NULL;
  	Real *target=NULL, *out=NULL, accu, itemaccu, avgaccu;
	
	fprintf(f,"%d\t%d", iter, examples->numExamples);
	accu=0.0;
	for(i=0;i<examples->numExamples;i++)
    	{ ex=&examples->examples[i];	// get each example;
      	  crbp_forward(net,ex);	// put to the network;
	  	
		  // initialize out and target;
		  out=malloc(PHO_OUT*sizeof(Real)); assert(out!=NULL); 
		  target=malloc(PHO_OUT*sizeof(Real)); assert(target!=NULL);
		  for(j=0;j<PHO_OUT;j++)
			{ out[j]=output->outputs[TIME-1][j];	// get output from the network;
			  target[j]=get_value(ex->targets,output->index,TIME-1,j);	// get target from the example;
			}
		  itemaccu=calaccu(out,target, v_method, v_thres);
		  fprintf(f,"\t%5.3f", itemaccu);
		  accu+=itemaccu;	// calculate accuracy;

		  free(out); free(target);	// release memory for out and target;
    	}
	avgaccu=accu/(float)(examples->numExamples);
	fprintf(f,"\t%5.3f\n", avgaccu);

  	return avgaccu;
}


void train(Net *net, ExampleSet *TrExm, ExampleSet *TeExm, int to, int step, int v_method, double v_thres, FILE *f1, FILE *f2, FILE *f3, char *weightF)
{ // train the network and record the training error and accuracies;
  	assert(net!=NULL); assert(TrExm!=NULL); assert(TeExm!=NULL); assert(f1!=NULL); assert(f2!=NULL); assert(f3!=NULL); assert(weightF!=NULL); 
	int i, j, iter, count;
  	Example *ex;
  	Real error, accuTr=0.0, accuTe=0.0;

	count=1; error=0.0;
  	for(iter=1;iter<=to;iter++)
  		{ ex=get_random_example(TrExm);
		  crbp_forward(net,ex);
    	  crbp_compute_gradients(net,ex);
    	  error+=compute_error(net,ex);
    	  bptt_apply_deltas(net);
          /* is it time to write status? */
    	  if(count==step)
			{ error=error/(float)count;
			  accuTr=getAccu(net, TrExm, iter, v_method, v_thres, f2);
			  accuTe=getAccu(net, TeExm, iter, v_method, v_thres, f3);
			  printf("iter=%d\terr=%5.3f\tacuTr=%5.3f\tacuTe=%5.3f\n", iter, error, accuTr, accuTe);	// display on screen;
	  		  fprintf(f1, "%d\t%d\t%d\t%5.3f\t%5.3f\t%d\t%5.3f\t%5.3f\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\n", iter, TIME, TAI, EPSI, INTCONST, ACTTYPE, ERRRAD, RANGE, OrthoS, HidS, PhonoS, PhoHidS, error, accuTr, accuTe);	// store parameters and results into f;
			  count=1; 
			  error=0.0; accuTr=0.0; accuTe=0.0;
		 	}
    	  else count++;
  		}
  	save_weights(net,weightF);
}


// main function;
void main(int argc,char *argv[])
{
  	int i, run, iseq, iter, rep, v_method, sum;
	Real v_thres;
  	long seed;
	FILE *f=NULL, *f1=NULL, *f2=NULL, *f3=NULL;
	char sep[]="/", *seedDirect=NULL, *subDirect=NULL, *locDirect=NULL;
	char *root=NULL, *outF=NULL, *weightF=NULL, *itemacuTrF=NULL, *itemacuTeF=NULL;
	
	printf("input subdic name(int): "); scanf("%d", &iseq); printf("subdic is %d\n", iseq);

	announce_version();
	setbuf(stdout,NULL); 
	
	load_phoneme(phoF);  // initialize phoneme;
	  
	seed=SEED+100*iseq; iter=ITER; rep=REP; v_method=V_METHOD; v_thres=V_THRES;
	// all the parameters can also be set by input arguments;
	for(i=1;i<argc;i++)
		{ if(strcmp(argv[i],"-seed")==0){ seed=atol(argv[i+1]); i++; }
		  else if(strncmp(argv[i],"-iter",5)==0){ iter=atoi(argv[i+1]); i++; }
		  else if(strncmp(argv[i],"-rep",4)==0){ rep=atoi(argv[i+1]); i++; }
		  else if(strncmp(argv[i],"-met",4)==0) { v_method=atoi(argv[i+1]); i++; }
		  else if(strncmp(argv[i],"-thres",6)==0) { v_thres=atof(argv[i+1]); i++; }
		}
	
	mikenet_set_seed(seed);
	build_model();	// build a network, with TIME number of time ticks; 
	sum=count_connections(reading); printf("connections: %d\n",sum);	// calculate number of connections and print out;

	training_examples=load_examples(exTrF, TIME); // load training examples;
	testing_examples=load_examples(exTeF, TIME);	// load testing examples;
	
	// handle subDirect, locDirect;
	subDirect=malloc((strlen("./")+2+(int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(subDirect!=NULL);
	locDirect=malloc(((int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(locDirect!=NULL);
	sprintf(locDirect, "%d", iseq); strcpy(subDirect, "./"); strcat(subDirect, locDirect); strcat(subDirect, sep);
	if(mkdir(subDirect, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1) { printf("can't create directory %s!\n", subDirect); exit(1); } // used in Linux;

	// save seed into seed.txt;
	seedDirect=malloc((strlen(subDirect)+strlen("seed.txt")+1)*sizeof(char)); assert(seedDirect!=NULL);
	strcpy(seedDirect, subDirect); strcat(seedDirect, "seed.txt");
	if((f=fopen(seedDirect,"w"))==NULL) { printf("Can't create %s\n", seedDirect); exit(1); } fprintf(f, "Seed=%lu\n", seed); fclose(f);	// store seed into seed.txt;
	free(seedDirect); seedDirect=NULL;

	// set up directories for result files;
	root=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(root!=NULL); 
	strcpy(root, subDirect);
	outF=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(outF!=NULL);
	strcpy(outF, root); strcat(outF, "output.txt");
	itemacuTrF=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(itemacuTrF!=NULL);
	strcpy(itemacuTrF, root); strcat(itemacuTrF, "itemacu_tr.txt");
	itemacuTeF=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(itemacuTeF!=NULL);
	strcpy(itemacuTeF, root); strcat(itemacuTeF, "itemacu_te.txt");
	weightF=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(weightF!=NULL);
	strcpy(weightF, root); strcat(weightF, "weights.txt");

	if((f1=fopen(outF,"w+"))==NULL) { printf("Can't open %s\n", outF); exit(1); }
	fprintf(f1, "ITER\tTICK\tTAI\tEPSI\tINTCONST\tACTTYPE\tERRRAD\tRANGE\tOrthS\tHidS\tPhonoS\tPhoHidS\tErr\tAcuTr\tAcuTe\n");
	
	if((f2=fopen(itemacuTrF,"a+"))==NULL) { printf("Can't open %s\n", itemacuTrF); exit(1); }
	fprintf(f2,"ITER\tNoItem");
	for(i=0;i<training_examples->numExamples;i++)
		fprintf(f2,"\tAcu%d",i+1);
	fprintf(f2,"\tAvg\n");

	if((f3=fopen(itemacuTeF,"a+"))==NULL) { printf("Can't open %s\n", itemacuTeF); exit(1); }
	fprintf(f3,"ITER\tNoItem");
	for(i=0;i<testing_examples->numExamples;i++)
		fprintf(f3,"\tAcu%d",i+1);
	fprintf(f3,"\tAvg\n");
	
	train(reading, training_examples, testing_examples, iter, rep, v_method, v_thres, f1, f2, f3, weightF);	// train network

	fclose(f1); fclose(f2);	//fclose(f3); // close result files;
	
	free(weightF); weightF=NULL; free(outF); outF=NULL;
	free(root); root=NULL; free(subDirect); subDirect=NULL; free(locDirect); locDirect=NULL;				
	  		  
	free(training_examples); training_examples=NULL;	// free training_examples;
	free(testing_examples); testing_examples=NULL;	// free testing_examples;
	free_net(reading); reading=NULL; // free network components;

	free(phon); phon=NULL;	// free phon;
}

