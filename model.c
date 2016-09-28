#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mikenet/simulator.h>

#include "model.h"

// functions to get parameter values;
void getpara_int(FILE *f, int *para)
{ // get integer paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atoi(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void getpara_double(FILE *f, double *para)
{ // get double paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atof(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void getpara_string(FILE *f, char **s)
{	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) 
		{ *s=malloc((strlen(token)+1)*sizeof(char)); assert(*s!=NULL); 
		  strcpy(*s, token); token=NULL; 
		}
	free(line); line=NULL;
	token=NULL;
}

void readpara(void)
{ // read parameters from para.txt;
	FILE *f=NULL; 
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);

	if((f=fopen("para.txt","r+"))==NULL) { printf("Can't open OverAllPara.txt\n"); exit(1); }

	fgets(line, _LineLen, f);	  // read: // Network Parameters
	getpara_int(f, &_tai);	// read _tai; default value is 1.
	getpara_int(f, &_tick);	// read _tick; number of ticks in one epoch (trial) of training; different types of training can happen in different ticks;
	getpara_double(f, &_intconst);	// read _intconst; default value is 0.25;
	getpara_double(f, &_epsi);	// read _epsi; episilon value for the activation curve; default value is 0.001;
	getpara_int(f, &_acttype);	// read _acttype; // LOGISTIC_ACTIVATION (0), TANH_ACTIVATION (1), FAST_LOGISTIC_ACTIVATION (2), LINEAR_ACTIVATION (3), or STEP_ACTIVATION (4); 
	getpara_int(f, &_errortype); // int _errortype; SUM_SQUARED_ERROR (1) or CROSS_ENTROPY_ERROR (2)
	getpara_double(f, &_errrad);	// read _errrad; error radius, errors less than it are counted as zero; default value is 0.1;
	getpara_double(f, &_range);	// read _range; range of initial weights, the initially randomized weights are positive and negative _range;
	getpara_int(f, &_OrthoS);	// read _OrthoS; size (number of nodes) of the orthographical layer;
	getpara_int(f, &_HidS);	// read _HidS; size of the hidden layer between the orthographical and phonological layers;
	getpara_int(f, &_PhonoS);	// read _PhonoS; size of the phonological layer; For Harm & Seidenberg 1999, 77 (7*11); For Harm 1998, 175 (7*25);
	getpara_int(f, &_PhoHidS);	// read _PhoHidS; size of the hidden layers between phonological layers, this is the cleanup layer;

	fgets(line, _LineLen, f);	// read: // Parameters for phonemes
	getpara_int(f, &_pho_features);	// read _pho_features; number of features in each phoneme; For Harm & Seidenberg 1999, 11; For Harm 1998, 25;
	getpara_int(f, &_pho_number);	// read _pho_number; number of phonemes in the dictionary; For Harm & Seidenberg 1999, 36 (24+11+1); For Harm 1998, 39 (24+10+4+1);

	fgets(line, _LineLen, f);	// read: // Parameters for file names storing phonological dictionary and training and testing examples
	getpara_string(f, &_phoF);	// read _phoF; file name of the phoneme dictionary, which is a list of phonemes and their feature values;
	getpara_string(f, &_exTrF);	// read _exTrF; file name of the training examples; TrEm_HarmSeidenberg1999.txt or TrEm_Harm1998.txt;
	getpara_string(f, &_exTeF);	// read _exTeF; file name of the testing examples; TeEm_HarmSeidenberg1999.txt or TeEm_Harm1998.txt or TE.txt (for nonwords); 

	free(line); line=NULL;
	fclose(f);
}

// function to build up the model
void build_model(void)
{ // build a network, with TIME number of time ticks
	readpara();

	// set up network parameters
	default_tai=_tai;
  	reading=create_net(_tick);
  	reading->integrationConstant=_intconst;

  	/* learning rate, activation pattern, and error method */
  	default_epsilon=_epsi;
  	default_activationType=_acttype;
	default_errorComputation=_errortype;

  	/* error radius */
  	default_errorRadius=_errrad;

  	/* create our groups. format is: name, num of units,  ticks */
  	input=init_group("Ortho",_OrthoS,_tick);
  	hidden=init_group("Hidden",_HidS,_tick);
  	output=init_group("Phono",_PhonoS,_tick);
  	phohid=init_group("PhoHid",_PhoHidS,_tick);

  	/* now add our groups to the network object */
  	bind_group_to_net(reading,input);
  	bind_group_to_net(reading,hidden);
  	bind_group_to_net(reading,output);
  	bind_group_to_net(reading,phohid);

  	/* now connect our groups, instantiating connection objects c1 through c4 */
  	c1=connect_groups(input,hidden);
  	c2=connect_groups(hidden,output);
  	c3=connect_groups(output,output);
  	c4=connect_groups(output,phohid);
  	c5=connect_groups(phohid,output);

  	/* add connections to our network */
  	bind_connection_to_net(reading,c1);
  	bind_connection_to_net(reading,c2);
  	bind_connection_to_net(reading,c3);
  	bind_connection_to_net(reading,c4);
  	bind_connection_to_net(reading,c5);

  	/* randomize the weights in the connection objects. Second argument is weight range. */
  	randomize_connections(c1,_range);
  	randomize_connections(c2,_range);
  	randomize_connections(c3,_range);
  	randomize_connections(c4,_range);
  	randomize_connections(c5,_range);

  	c3->epsilon=_epsi; 
	c4->epsilon=_epsi; 
	c5->epsilon=_epsi;

	int i;
  	precompute_topology(reading,input);
  	for(i=0;i<reading->numGroups;i++)
  		printf("%s %d\n",reading->groups[i]->name,reading->groups[i]->whenDataLive);
  
  	for(i=0;i<c3->to->numUnits;i++)
    	{ // freeze c3 connection weights to 0.75!
      	  c3->weights[i][i]=0.75;	 
      	  c3->frozen[i][i]=1;
    	}
}

int count_connections(void)
{ // calculate number of connections in the network;
	assert(reading!=NULL);
  	int i, j, k, count=0;
  	for(i=0;i<reading->numConnections;i++)
    	count += (reading->connections[i]->from->numUnits)*(reading->connections[i]->to->numUnits);
  	return count;
}

void load_phoneme(char *PhoF)
{ // initialize phon by reading from PhoF;
	assert(PhoF!=NULL);
	int curphon, curvec;
	FILE *f=NULL;
	char line[_LineLen], *p=NULL;
	// initialize PhoDic;
	_phon=malloc(_pho_number*sizeof(Phoneme)); assert(_phon!=NULL);
	// read from PhoF;
	if((f=fopen(PhoF,"r"))==NULL){ printf("Can't open %s\n", PhoF); exit(1); }
	curphon=0;
	while(!feof(f))
    	{ fgets(line, _LineLen, f);
		  _phon[curphon].name=strtok(line," ")[0];
		  _phon[curphon].vec=malloc(_pho_features*sizeof(Real)); assert(_phon[curphon].vec!=NULL);
		  curvec=0;
		  while(p=strtok(NULL, " "))
			{ _phon[curphon].vec[curvec]=atof(p); 
			  curvec++;
			}
		  curphon++;
		}
	fclose(f);
	/*
	// print PhoDic
	int i, j;
	for(i=0;i<_pho_number;i++)
		{ printf("Phoneme %c: ", _phon[i].name);
		  for(j=0;j<_pho_features;j++)
	  		printf("%2.1f ", _phon[i].vec[j]);
	     	  printf("\n");
		}
	*/
}

void delete_phoneme(void)
{ // delete phon;
	int i;
	for(i=0;i<_pho_number;i++)
		{ free(_phon[i].vec); _phon[i].vec=NULL;
		}
	free(_phon); _phon=NULL;
}
