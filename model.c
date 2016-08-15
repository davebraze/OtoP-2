#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mikenet/simulator.h>

#include "model.h"

// functions to get parameter values;
void GetUnsignedIntPara(FILE *ParaFile, unsigned int *para)
{	assert(ParaFile!=NULL);
	int sizeline=_LineLen;
	char *line=malloc(sizeline*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, sizeline, ParaFile); token=strtok(line, sep);
   	while(token!=NULL) { *para=atoi(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void GetIntPara(FILE *f, int *para)
{ // get integer paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atoi(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void GetDoublePara(FILE *f, double *para)
{ // get double paramter;
	assert(f!=NULL);
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);
	char sep[]="\t", *token=NULL;

	fgets(line, _LineLen, f); token=strtok(line, sep);
   	while(token!=NULL) { *para=atof(token); token=NULL; }
	free(line); line=NULL;
	token=NULL;
}

void GetStringPara(FILE *f, char **s)
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

void readPara(void)
{ // read parameters from para.txt;
	FILE *f=NULL; 
	char *line=malloc(_LineLen*sizeof(char)); assert(line!=NULL);

	if((f=fopen("para.txt","r+"))==NULL) { printf("Can't open OverAllPara.txt\n"); exit(1); }

	fgets(line, _LineLen, f);	  // read: // Network Parameters
	GetIntPara(f, &_tai);	// read _tai;
	GetIntPara(f, &_tick);	// read _tick;
	GetDoublePara(f, &_intconst);	// read _intconst;
	GetDoublePara(f, &_epsi);	// read _epsi;
	GetIntPara(f, &_acttype);	// read _acttype; // TANH_ACTIVATION
	GetDoublePara(f, &_errrad);	// read _errrad;
	GetDoublePara(f, &_range);	// read _range;
	GetIntPara(f, &_OrthoS);	// read _OrthoS;
	GetIntPara(f, &_HidS);	// read _HidS;
	GetIntPara(f, &_PhonoS);	// read _PhonoS;
	GetIntPara(f, &_PhoHidS);	// read _PhoHidS;

	fgets(line, _LineLen, f);	// read: // Phoneme library and example file
	GetIntPara(f, &_pho_features);	// read _pho_features;
	GetIntPara(f, &_pho_number);	// read _pho_number;
	GetIntPara(f, &_pho_out);	// read _pho_out;
	GetStringPara(f, &_phoF);	// read _phoF;
	GetStringPara(f, &_exTrF);	// read _exTrF;
	GetStringPara(f, &_exTeF);	// read _exTeF;

	free(line); line=NULL;
	fclose(f);
}

// function to build up the model
void build_model(void)
{ // build a network, with TIME number of time ticks
	readPara();

	// set up network parameters
	default_tai=_tai;
  	reading=create_net(_tick);
  	reading->integrationConstant=_intconst;

  	/* learning rate */
  	default_epsilon=_epsi;
  	default_activationType=_acttype;

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


	// finally, show network parameters to make sure they are correctly read and assigned!
	printf("Network Parameters:\n");
	printf("TAI=%d, TICK=%d;\n", default_tai, _tick);
	printf("INTCONST=%f, EPSI=%f, ACTTYPE=%d, ERRRAND=%f, RANGE=%f\n", reading->integrationConstant, default_epsilon, default_activationType, default_errorRadius, _range);
	printf("OrthoS=%d, HidS=%d, PhonoS=%d, PhoHidS=%d\n", _OrthoS, _HidS, _PhonoS, _PhoHidS);
	printf("Pho_feature=%d, Pho_number=%d, Pho_out=%d\n", _pho_features, _pho_number, _pho_out);
	printf("PhonF: %s, TrainF: %s, TestF: %s\n", _phoF, _exTrF, _exTeF);
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
	
	// print PhoDic
	int i, j;
	for(i=0;i<_pho_number;i++)
		{ printf("Phoneme %c: ", _phon[i].name);
		  for(j=0;j<_pho_features;j++)
	  		printf("%2.1f ", _phon[i].vec[j]);
	     	  printf("\n");
		}
}

void delete_phoneme(void)
{ // delete phon;
	int i;
	for(i=0;i<_pho_number;i++)
		{ free(_phon[i].vec); _phon[i].vec=NULL;
		}
	free(_phon); _phon=NULL;
}
