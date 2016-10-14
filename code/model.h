#define _LineLen 300

// network components
Net *reading;
Group *input,*hidden,*output,*phohid;
Connections *c1,*c2,*c3,*c4,*c5;
ExampleSet *train_exm,*test_exm,*train_ptop_exm;

// network parameters
int _tick_OtoP,_tick_PtoP;
double _intconst;
int _tai;
double _epsi;
int _acttype,_errortype;
int _weightnoisetype; 
double _weightnoise,_actnoise,_inputnoise;
double _errrad,_range;
int _OrthoS,_HidS,_PhonoS,_PhoHidS;

// parameters for phonemes
int _pho_features, _pho_number;
typedef struct
{ char name;	// name of phoneme;
  Real *vec;	// features of phoneme;
} Phoneme;
Phoneme *_phon;

// parameters for file names storing phonological dictionary and training and testing examples;
char *_phoF;
char *_exTrF,*_exTeF,*_exTrF_PtoP;

// function to build the network and phoneme dictionary;
void build_reading_model(int ticks);
int count_connections(Net *net);
void load_phoneme(char *PhoF);
void delete_phoneme(void);