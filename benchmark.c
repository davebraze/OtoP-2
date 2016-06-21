#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <mikenet/simulator.h>
#include <time.h>
#include <assert.h>
#include<sys/stat.h>
#include<sys/types.h>

#include "model.h"

// parameters for phonemes
#define PHO_FEATURES 11
#define PHO_NUMBER 35
#define PHO_OUT 55
#define VECT_THRES 0.5
#define VECT_METHOD 0
#define phoFileName "./phonemes.txt"
#define exFileName "./examples.txt"

typedef struct
{
  char name;	// name of phoneme;
  Real vec[PHO_FEATURES];	// features of phoneme;
} Phoneme;
Phoneme *phon=NULL;

// parameters for recording timepoints, total iteration, and seed;
#define RUNNUM 5
#define SEED (long)(time(NULL))
#define REP 500
#define ITER 5000

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


int vect_check(Real *x1, Real *x2)
{ // check vector's bit-based threshold between two vectors x1 and x2;
	assert(x1!=NULL); assert(x2!=NULL);
	int i, inThres=1;
	for(i=0;i<PHO_FEATURES;i++)
		{ if(abs(x1[i]-x2[i])>=VECT_THRES) { inThres=0; break;}
		}
	return inThres;
}


void FindPhoneme(Real *vect, Real *trans)
{ // find phoneme matching vect; 
	assert(vect!=NULL); assert(trans!=NULL); 
	int i, j, ind, MaxDist, numInThres, *InThresSet=NULL, curind;
	
	if(VECT_METHOD==0)
		{ // using phoneme with smallest Euclidean distance to vect as trans;
		  ind=-1; MaxDist=1e6;
		  for(i=0;i<PHO_NUMBER;i++)
			{ if(euclid_dist(vect,phon[i].vec)<=MaxDist) { ind=i; MaxDist=euclid_dist(vect,phon[i].vec); }
			}
		  assert(ind!=-1);
		  for(i=0;i<PHO_FEATURES;i++)
			trans[i]=phon[ind].vec[i];
		}
	else if(VECT_METHOD==1)
		{ // using phoneme with 0.5 threshold to set trans;
		  numInThres=0;
		  for(i=0;i<PHO_NUMBER;i++)
			{ if(vect_check(vect,phon[i].vec)) numInThres++;
			}
		  if(numInThres==0)
			{ // no such phoneme;
		  	  for(i=0;i<PHO_FEATURES;i++)
				trans[i]=-2.0;
			}
		  else if(numInThres==1)
			{ // there is only one phoneme that matches this requirement;
			  for(i=0;i<PHO_NUMBER;i++)
				{
				  if(vect_check(vect,phon[i].vec))
					{ 
					  for(j=0;j<PHO_FEATURES;j++)
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
				{ if(vect_check(vect,phon[i].vec)) { InThresSet[curind]=i; curind++; }
				}
			  ind=Rand0_1*numInThres;
			  for(i=0;i<PHO_FEATURES;i++)
				trans[i]=phon[InThresSet[ind]].vec[i];	
			  free(InThresSet); InThresSet=NULL;
			}
		}
}


Real calaccu(Real *out, Real *target)
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
		  FindPhoneme(vect, trans);
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


Real getAccu(Net *net, ExampleSet *examples)
{ // calculate accuracy of the network;
	assert(net!=NULL); assert(examples!=NULL);
	int i, j;
	Example *ex=NULL;
  	Real *target=NULL, *out=NULL, accu=0.0;
	
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
		  accu+=calaccu(out,target);	// calculate accuracy;
		  free(out); free(target);	// release memory for out and target;
    	}
  	return accu/(float)(examples->numExamples);
}


void train(Net *net, ExampleSet *examples, int to, int step, char *outputFileName, char *weightFileName)
{ // train the network and record the training error and accuracies;
  	assert(net!=NULL); assert(examples!=NULL); assert(outputFileName!=NULL); assert(weightFileName!=NULL); 
	int i, j, iter, count;
  	Example *ex;
  	Real error, accu;
  	FILE *f=NULL;

  	if((f=fopen(outputFileName,"w+"))==NULL) { printf("Can't open %s\n", outputFileName); exit(1); }

	count=1; error=0.0; accu=0.0;
  	for(iter=1;iter<=to;iter++)
  		{ ex=get_random_example(examples);
    	  crbp_forward(net,ex);
    	  crbp_compute_gradients(net,ex);
    	  error+=compute_error(net,ex);
    	  bptt_apply_deltas(net);
      
    	  /* is it time to write status? */
    	  if(count==step)
			{ error=error/(float)count; 
			  accu=getAccu(net, examples);
			  printf("iter=%d\terror=%5.3f\taccu=%5.3f\tepsi=%5.3f\n", iter, error, accu, EPSILON);
	  		  fprintf(f, "%d\t%5.3f\t%5.3f\t%5.3f\n", iter, error, accu, EPSILON);
	  		  count=1; 
			  error=0.0; accu=0.0;
		 	}
    	  else count++;
  		}
  	save_weights(net,weightFileName);

	fclose(f);
}


// main function;
int main(int argc,char *argv[])
{
  	int i, run, iseq, iter, rep, sum;
  	long seed;
	FILE *f=NULL;
	char sep[]="/", *seedDirect=NULL, *subDirect=NULL, *locDirect=NULL, *root=NULL, *outFileName=NULL, *weightFileName=NULL;
	
	setbuf(stdout,NULL); announce_version();

	if(RUNNUM!=0)
		{ // serial mode
		  for(run=0;run<RUNNUM;run++)
		  	{ seed=SEED; iter=ITER; rep=REP;
		 	  // seed and iteration number can also be set by input arguments;
		  	  for(i=1;i<argc;i++)
				{ if(strcmp(argv[i],"-seed")==0){ seed=atol(argv[i+1]); i++; }
				  else if(strncmp(argv[i],"-iter",5)==0){ iter=atoi(argv[i+1]); i++; }
				  else if(strncmp(argv[i],"-rep",4)==0){ rep=atoi(argv[i+1]); i++; }
				}
		  	  mikenet_set_seed(seed);

			  build_model();	// build a network, with TIME number of time ticks; 
		  	  sum=count_connections(reading); printf("connections: %d\n",sum);	// calculate number of connections and print out;
		
		  	  load_phoneme(phoFileName);	// initialize phoneme;	
		  	  reading_examples=load_examples(exFileName, TIME);	// load examples;

			  subDirect=malloc((strlen("./")+2+(int)(log10((double)(RUNNUM))+1)+1)*sizeof(char)); assert(subDirect!=NULL);
		  	  locDirect=malloc(((int)(log10((double)(RUNNUM))+1)+1)*sizeof(char)); assert(locDirect!=NULL);
		  	  sprintf(locDirect, "%d", run+1); strcpy(subDirect, "./"); strcat(subDirect, locDirect); strcat(subDirect, sep);
		  	  if(mkdir(subDirect, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1) { printf("can't create directory %s!\n", subDirect); exit(1); } // used in Linux;
		
	 	  	  // save seed into seed.txt;
		  	  seedDirect=malloc((strlen(subDirect)+strlen("seed.txt")+1)*sizeof(char)); assert(seedDirect!=NULL);
		  	  strcpy(seedDirect, subDirect); strcat(seedDirect, "seed.txt");
		  	  if((f=fopen(seedDirect,"w"))==NULL) { printf("Can't create %s\n", seedDirect); exit(1); } fprintf(f, "Seed=%lu\n", seed); fclose(f);	// store seed into seed.txt;
		  	  free(seedDirect); seedDirect=NULL;

			  printf("Run=%*d; Seed=%lu\n", (int)(log10((double)(RUNNUM))+1), run+1, seed); // display current run and seed;

			  // set up root directory for storing result files;
		  	  root=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(root!=NULL); strcpy(root, subDirect);
		  	  outFileName=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(outFileName!=NULL);
		  	  strcpy(outFileName, root); strcat(outFileName, "output.txt");
		  	  weightFileName=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(weightFileName!=NULL);
		  	  strcpy(weightFileName, root); strcat(weightFileName, "weights.txt");
		
		  	  train(reading, reading_examples, iter, rep, outFileName, weightFileName);	// train network
		
		  	  free(weightFileName); weightFileName=NULL; free(outFileName); outFileName=NULL;
		  	  free(root); root=NULL; free(subDirect); subDirect=NULL; free(locDirect); locDirect=NULL;
				
		  	  free(phon); phon=NULL;
			  free_model();	// free memory space for network components;
		  	}
		}
	else if(RUNNUM==0)
		{ // parallel run; create particular subdirectory; for hpc power computers; 
		  seed=SEED; iter=ITER; rep=REP;
		  // seed and iteration number can also be set by input arguments;
		  for(i=1;i<argc;i++)
			{ if(strcmp(argv[i],"-seed")==0){ seed=atol(argv[i+1]); i++; }
			  else if(strncmp(argv[i],"-iter",5)==0){ iter=atoi(argv[i+1]); i++; }
			  else if(strncmp(argv[i],"-rep",4)==0){ rep=atoi(argv[i+1]); i++; }
			}
		  mikenet_set_seed(seed);
		
		  build_model();	// build a network, with TIME number of time ticks; 
		  sum=count_connections(reading); printf("connections: %d\n",sum);	// calculate number of connections and print out;
		
		  load_phoneme(phoFileName);	// initialize phoneme;	
		  reading_examples=load_examples(exFileName, TIME);	// load examples;
		
		  printf("input iseq: "); scanf("%d", &iseq); printf("iseq is %d\n", iseq);
		  subDirect=malloc((strlen("./")+2+(int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(subDirect!=NULL);
		  locDirect=malloc(((int)(log10((double)(iseq))+1)+1)*sizeof(char)); assert(locDirect!=NULL);
		  sprintf(locDirect, "%d", iseq); strcpy(subDirect, "./"); strcat(subDirect, locDirect); strcat(subDirect, sep);
		  if(mkdir(subDirect, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1) { printf("can't create directory %s!\n", subDirect); exit(1); } // used in Linux;
		
	 	  // save seed into seed.txt;
		  seedDirect=malloc((strlen(subDirect)+strlen("seed.txt")+1)*sizeof(char)); assert(seedDirect!=NULL);
		  strcpy(seedDirect, subDirect); strcat(seedDirect, "seed.txt");
		  if((f=fopen(seedDirect,"w"))==NULL) { printf("Can't create %s\n", seedDirect); exit(1); } fprintf(f, "Seed=%lu\n", seed); fclose(f);	// store seed into seed.txt;
		  free(seedDirect); seedDirect=NULL;
				
		  // set up root directory for storing result files;
		  root=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(root!=NULL); strcpy(root, subDirect);
		  outFileName=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(outFileName!=NULL);
		  strcpy(outFileName, root); strcat(outFileName, "output.txt");
		  weightFileName=malloc((strlen(subDirect)+2+_FileLen)*sizeof(char)); assert(weightFileName!=NULL);
		  strcpy(weightFileName, root); strcat(weightFileName, "weights.txt");
		
		  train(reading, reading_examples, iter, rep, outFileName, weightFileName);	// train network
		
		  free(weightFileName); weightFileName=NULL; free(outFileName); outFileName=NULL;
		  free(root); root=NULL; free(subDirect); subDirect=NULL; free(locDirect); locDirect=NULL;
				
		  free(phon); phon=NULL;
		  free_model();	// free network components;
		}	
	return 0;
}

