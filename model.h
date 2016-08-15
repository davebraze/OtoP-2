#define _LineLen 300

// network components
Net *reading;
Group *input,*hidden,*output,*phohid;
Connections *c1,*c2,*c3,*c4,*c5;
ExampleSet *train_exm, *test_exm;

// network parameters
int _tai, _tick;
double _intconst, _epsi;
int _acttype; 
double _errrad, _range;
int _OrthoS, _HidS, _PhonoS, _PhoHidS;

// parameters for phonemes
int _pho_features, _pho_number, _pho_out;
typedef struct
{ char name;	// name of phoneme;
  Real *vec;	// features of phoneme;
} Phoneme;
Phoneme *_phon;
char *_phoF, *_exTrF, *_exTeF;

// functions to get parameter values;
void GetUnsignedIntPara(FILE *f, unsigned int *para);
void GetIntPara(FILE *f, int *para);
void GetDoublePara(FILE *f, double *para);
void GetStringPara(FILE *f, char **s);
void readPara(void);

// function to build the network;
void build_model(void);
int count_connections(void);
void load_phoneme(char *PhoF);
void delete_phoneme(void);