# Document Use of Subset\_CocaMoby.py for Derivation of OtoP Dictionary

This file documents use of Subset\_CocaMoby.py for the purpose of
building a training dictionary for use in an OtoP learning simulation
built in MikeNet. Dictionary entries, ortho/phono pairs together with
frequency weights, are derived from the CoCa and Moby
databases. Frequency information comes from CoCa
<http://corpus.byu.edu/coca/>, and O-to-P mappings from the Moby
Pronunciator <http://icon.shef.ac.uk/Moby/>.

Words selected for inclusion in the training dictionary are
monosyllabic and (mostly) mono-morphemic.

We incorporate some simple transformations to the Moby pronunciations
in order to facilitate use in a Mikenet framework.

## Details of steps from Moby/CoCa to MikeNet OtoP Dictionary
1. Read from Moby pronunciator and make some replacements in the phonemes.
   These replacements include:
   1. delete the glide '/-/' after the vowel '/aI/'
   2. replace '/-/' with '/@/'
   3. replace '/hw/' with 'w'
   4. replace 'R' with 'r'
   5. replace '/(@)/' and '/[@]/' with '/@/'
   6. remove pronunications with '/x/'
2. Read modified Moby pronunciator after 1) and select pronunciations having at most 1 vowel
   The results are stored in subMobypron\_dict.csv
3. Read Coca database and select words having at most 8 letters. There are two result csvs based
   on different Coca database: subCoca\_dict1.csv is based on the lemma words' frequencies;
   subCoca\_dict2.csv is based on real word forms' frequencies;
4. Replace pronunciations of Moby to pronunciations of Benchmark. Replacement includes:
   1. replace notations of consonants and vowels;
	  for consonants: '/tS/'->'C'; '/dZ/'->'J'; '/S/'->'S'; '/T/'->'T'; '/D/'->'D'; '/Z/'->'B'; '/N/'->'G'

	  for vowels: '/i/'->'i'; '/I/'->'I'; '/E/'->'E'; '/&/'->'@'; '/A/'->'a'; '/O/'->'a'; '/(@)/'->'E'; '/oU/'->'o';
              '/U/'->'U'; '/u/'->'u'; '/@/'->'^'

   2. replace and separate diphthongs into single vowel plus consonant:
   '/eI/'->'e/j'; '/aI/'->'a/j'; '/Oi/'->'o/j'; '/AU/'->'a/w'

	Results are stored respectively in extwords1.csv (based on subCoca\_dict1.csv)
    and extwords2.csv (based on subCoca\_dict2.csv)

	In extword1.csv and extword2.csv, column 'wordform' is word form, column 'sum\_freq' is word frequency
   in Coca, column 'norm\_freq' is normalized frequency per million (dividing 'sum\_freq' by 450),
   column 'Moby\_pron' is Moby pronunciation of the word, column 'Rep\_P' is phonological representation of the word,
   column 'Rep\_O' is orthographical representation of the word, '/' separates phonemes or letters.
   In extword2.csv, column 'word\_class' is class type of each word
5. Create extword3.csv based on extword2.csv, by summing up the same word form's frequencies across all word types.
6. Generate training example files based on extword1.csv, extword2.csv and extword3.csv, the results are:
   trainexp\_full1.txt, trainexp\_full2.txt, and trainexp\_full3.txt
