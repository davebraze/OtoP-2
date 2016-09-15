library(stringr)
library(reshape2)
library(tidyr)
library(ggplot2)
library(hexbin)
library(plyr)

options(max.print=10000)

##################################################
## Get training data (O to P mappings).

# # read from extwords.csv
# fileNam <- 'extwords3_Harm1998.csv'
# words <- read.csv(fileName)
# nms <- str_replace(names(words), "_", ""); nms <- str_to_lower(nms)
# names(words) <- nms
# P <- str_replace_all(words$repp, "(_|/)", "")
# OP <- paste(words$wordform, P, sep=".")

## Get O,P pairs and frequency from mikenet training set.
# getDict <- function(fname) {
#   val <- readLines(fname, n=-1)
#   val <- val[stringr::str_detect(val, "^(TAG|PROB) ")]
#   idx <- rep(1:(length(val)/2), each=2)
#   dat <- data.frame(val,idx)
#   cleanD <- function(d) {
#     O <- stringr::str_extract(d[1,]$val, "Rep_O: [^ ]+")
#     O <- stringr::str_replace_all(O, "(Rep_O:|[ /_])", "")
#     P <- stringr::str_extract(d[1,]$val, "Rep_P: [^ ,]+")
#     P <- stringr::str_replace_all(P, "(Rep_P:|[ /_])", "")
#     prob <- stringr::str_extract(d[2,]$val, "[.0-9]+")
#     data.frame(O, P, prob)
#   }
#   plyr::ddply(dat, .(idx), cleanD)
# }
# 
# ff <- "./data/H100L1e-2/trainexp_full3.txt"
# OP <- getDict(ff)
# head(OP)

# read from TrEm.txt
getList <- function(te, exp1, exp2){
  raw <- str_extract(te, exp1); raw <- str_replace(raw, exp2, "")
  rawLoc <- which(!is.na(raw))
  rawList <- c()
  for(ll in rawLoc){
    rawList <- c(rawList, raw[ll])
  }
  return(rawList)
}

fileNam <- 'TrEm3_Harm1998.txt'; te <- readLines(fileNam)
head(te, 50)

wordList <- getList(te, "Word: ([a-z]+)", "Word: ") # get words
probList <- getList(te, "PROB ([0-9.]+)", "PROB ") # get probs
phonoList <- getList(te, "Rep_P: ([_/a-zA-Z@&^]+)", "Rep_P: ") # get phonological representations
orthoList <- getList(te, "Rep_O: ([_/a-zA-Z]+)", "Rep_O: ") # get orthographical representations
P <- str_replace_all(phonoList, "(_|/)", ""); OPList <- paste(wordList, P, sep=".") # create OP list

numWord <- length(unique(wordList)) # 4008 all O forms are unique
numP <- length(unique(P)) # 2799, but not so P
numOP <- length(unique(OPList)) # 4008
## there are no homographs, but there are homophones.

DF <- data.frame(word=wordList, prob=as.numeric(probList), 
                 phono=phonoList, ortho=orthoList, 
                 op=OPList)


##################################################
## get overall (average) performance data from a set of models. Model
##  outputs ("output.txt" files) are assumed to be in subdirectories
##  relative to the current working directory.
flist <- dir(".", pattern="^output.txt$", recursive=TRUE)

readOutput <- function(f){
  print(f)
  prs <- str_split(f, "/", simplify=TRUE) # prs[1] is condition; prs[2] is run id; prs[3] is filename (output.txt)
  hlsize <- as.integer(str_extract(str_split(prs[1], "L", simplify=TRUE)[1], "[0-9]+"))
  lrnrate <- as.numeric(str_split(prs[1], "L", simplify=TRUE)[2])
  run <- as.integer(prs[2])
  retval <- read.delim(f)
  retval <- data.frame(hlsize, lrnrate, run, retval)
}

tr <- ldply(flist, readOutput)
names(tr) <- str_to_lower(names(tr))

## format numbers in scientific notation
scinot <- function(x){
  if(is.numeric(x)){
    format(x, scientific=TRUE)
  }else{
    error("x must be numeric")
  }
}

ggplot(tr, aes(x=iter, y=acutr)) + scale_x_log10(labels=scinot) +  
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  geom_point(alpha=.2, color="blue") + geom_smooth(span=.2, color="darkorange") +
  facet_grid(lrnrate~hlsize)
ggsave('AvgAcu.png', dpi = 300, height = 6, width = 12, units = 'in')


##################################################
## get word-level performance data from a set of models & tidy it. 
## Data files ("itemacu_tr.txt") are assumed to be in
## subdirectories relative to the current working directory.

## read accuracy data
flist <- dir(".", pattern="^itemacu_tr.txt$", recursive=TRUE)
tr1 <- ldply(flist, readOutput)
names(tr1) <- str_to_lower(names(tr1))
tr1 <- tr1[-which(names(tr1) == "noitem")] # tidyr way to do this is ??

## re-label item columns, with wordforms (O.P) they represent
wrd <- which(str_detect(names(tr1), "^acu[0-9]+$")) # find the right columns
## should probably double check to ensure length(wrd) == length(OP)
## before proceeding
names(tr1)[wrd] <- OPList # NB: wordforms (O-rep) are unique

## convert from wide to long format
tr2 <- tidyr::gather(tr1, wrd, key="OP", value="accuracy")
tr2 <- tidyr::separate(tr2, OP, into=c("O", "P"), sep="[.]")

## read activated phoneme data
flist <- dir(".", pattern="^outphon.txt$", recursive=TRUE)
actphon <- ldply(flist, readOutput)
names(actphon) <- str_to_lower(names(actphon))
actphon <- actphon[-which(names(actphon) == "noitem")] # tidyr way to do this is ??

## re-label item columns, with wordforms (O.P) they represent
actp <- which(str_detect(names(actphon), "^phon[0-9]+$")) # find the right columns
## before proceeding
names(actphon)[actp] <- OPList # NB: wordforms (O-rep) are unique

## convert from wide to long format
tr2actp <- tidyr::gather(actphon, actp, key="OP", value="actp")
tr2actp <- tidyr::separate(tr2actp, OP, into=c("O", "P"), sep="[.]")
tr2actp$actp <- gsub("_", "", tr3$actphon)

# merge tr2 with tr2phon
tr2$actp <- tr2actp$actp

## subsample accuracy outputs ?
if(FALSE){
  samps <- sort(unique(tr1$iter))
  period <- 4 ## keep every nth sample
  keep <- c(TRUE, rep(FALSE, length.out=period-1))
  keep <- rep(keep, length.out=length(samps))
  tr1s <- subset(tr1, subset=keep)
}else{
  tr1s <- tr1
}

##### Plot average accuracy as output by model
tmp <- tr2[,c("hlsize", "lrnrate", "iter", "avg")]
tmp <- unique(tmp)
ggplot(tmp, aes(x=iter, y=avg)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  geom_point(alpha=.2, color="blue") + geom_smooth(span=.2, color="darkorange") +
  facet_grid(lrnrate~hlsize)


##### Diffs based on phonological structure
vowo <- c("a", "e", "i", "o", "u")

## Based on Harm & Seidenberg 1999
# vshort <- c("I", "E", "@", "a", "U", "^") # short vowels
# vlong <- c("i", "ej", "aj", "o", "u") # long vowels
# vdph <- c("aw", "oj") # diphthongs
# vowp <- c(vshort, vlong, vdph) # all vowels
# vowp.re <- paste0("(", paste(vowp, collapse="|"), ")")

## Based on Harm 1998
vowp <- c("i", "I", "E", "@", "^", "o", "U", "u", "e", "a", "W", "Y", "A", "O") # all vowels
vowp.re <- paste0("(", paste(vowp, collapse="|"), ")")

consp <- c("b", "d", "g", "p", "t", "k",
           "f", "T", "s", "S", "v", "D", "z", "Z",
           "C", "G",
           "j", "w",
           "m", "n", "N", "l", "r")
consp.re <- paste0("[", paste(consp, collapse=""), "]")

## find phon forms that are CVC with simple vowels. This def will also
## include CV, where V is a diphthong (e.g., pay, buy).
cvc.re <- paste0("^", consp.re, "{1}", vowp.re, "{1}", consp.re, "{1}", "$")
ccvc.re <- paste0("^", consp.re, "{2}", vowp.re, "{1}", consp.re, "{1}", "$")
cvcc.re <- paste0("^", consp.re, "{1}", vowp.re, "{1}", consp.re, "{2}", "$")
ccvcc.re <- paste0("^", consp.re, "{2}", vowp.re, "{1}", consp.re, "{2}", "$")

Pcvc <- str_detect(tr2$P, cvc.re)
Pccvc <- str_detect(tr2$P, ccvc.re)
Pcvcc <- str_detect(tr2$P, cvcc.re)
Pccvcc <- str_detect(tr2$P, ccvcc.re)
syl <- rep(NA, length.out=length(Pcvc))
syl[Pcvc] <- "cvc"
syl[Pccvc] <- "ccvc"
syl[Pcvcc] <- "cvcc"
syl[Pccvcc] <- "ccvcc"
tr2s <- data.frame(tr2, syl)
tr2sub <- subset(tr2s, !is.na(syl))

numCVC <- length(unique(tr2sub$O[tr2sub$syl=="cvc"])) # 1058
numCVCC <- length(unique(tr2sub$O[tr2sub$syl=="cvcc"])) # 489
numCCVC <- length(unique(tr2sub$O[tr2sub$syl=="ccvc"])) # 618
numCCVCC <- length(unique(tr2sub$O[tr2sub$syl=="ccvcc"])) # 170

ggplot(tr2sub, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +
  ggtitle("Phonological CVC vs. CCVC vs. CVCC vs. CCVCC") +
  geom_smooth(aes(color=as.factor(syl)), span=.2) +
  facet_grid(lrnrate~hlsize)
ggsave('CVCAcu.png', dpi = 300, height = 6, width = 12, units = 'in')


##### split items randomly into 4 groups
doGroups <- function(d){
  group <- sample(LETTERS[1:4], 1)
  data.frame(d, group)
}
tr2sr <- ddply(tr2s, "O", doGroups) ## will create a different random
## grouping each time this is run

windows()
ggplot(tr2sr, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +
  geom_smooth(aes(color=group), span=.2) + 
  #geom_smooth(color="black", aes(y=avg)) +
  ggtitle("Randomly Grouped") + 
  facet_grid(lrnrate~hlsize)
## compare a few of these to calibrate the eye

doGroupsN <- function(run, df){
  data.frame(run, ddply(df, "O", doGroups))
}
runs <- 1:4
tr2sr4 <- ldply(runs, doGroupsN, tr2s)

windows()
ggplot(tr2sr4, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +
  geom_smooth(aes(color=group), span=.2) + 
  #geom_smooth(color="black", aes(y=avg)) +
  facet_wrap(~run, nrow=2) + ggtitle("Randomly Grouped, 4 ways") + 
  facet_grid(lrnrate~hlsize)
##### break out sensible word groups to look at performance in more
##### detail

## length based categories
Olen <- str_length(tr2s$O) ## letter length
Plen <- str_length(tr2s$P) ## phoneme length

tr2sa <- data.frame(tr2s, Olen, Plen)
tr2sa <- data.frame(tr2sa, matchlen=Olen==Plen)

ggplot(tr2sa, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  #    coord_trans(limy=c(.3, .65)) +
  geom_smooth(aes(color=matchlen), span=.2) +
  #geom_smooth(color="black", aes(y=avg)) +
  ggtitle("O length == P length vs. O length != P length") + 
  facet_grid(lrnrate~hlsize)


tr3sa <- data.frame(tr2sa, difflen=Olen-Plen)
ggplot(tr3sa, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  geom_smooth(aes(color=as.factor(difflen)), span=.2) +
  #geom_smooth(color="black", aes(y=avg)) +
  ggtitle("O length minus P length") + 
  facet_grid(lrnrate~hlsize)
ggsave('Acu_OminusP.png', dpi = 300, height = 6, width = 12, units = 'in')

## Words with fewer letters than phonemes are the easiest cases to
## learn (e.g., ax, fix, next, fry, sky). Words that match in O and P
## length are the next easiest, and those with 1 fewer phones than
## letters are next. Some bins are small; see the confidence
## intervals.

ggplot(tr3sa, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  geom_smooth(aes(color=as.factor(Plen)), span=.2) +
  #geom_smooth(color="black", aes(y=avg)) +
  ggtitle("Length in Phonemes") + 
  facet_grid(lrnrate~hlsize)
ggsave('PlengAcu.png', dpi = 300, height = 6, width = 12, units = 'in')
## Relationship between word length (in phonemes) and
## accuracy. Shorter words are learned less well. That's just plain
## weird.


## Words with C that are often written as digraphs (e.g., ship, chip,
## judge, the, this) vs. those without such C.
digr.re <- "[TSDCG]"
Pdigr <- str_detect(tr3sa$P, digr.re)
tr3sa <- data.frame(tr3sa, Pdigr)

ggplot(tr3sa, aes(x=iter, y=accuracy)) + scale_x_log10(labels=scinot) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  geom_smooth(aes(color=as.factor(Pdigr)), span=.2) +
  #geom_smooth(color="black", aes(y=avg)) +
  ggtitle("Probable Digraph C vs. Other") + 
  facet_grid(lrnrate~hlsize)
## Digraphs are harder to learn. Not surprising.

# get words' frequencies
wordDF <- read.csv("extwords3_Harm1998.csv")
median_log_freq <- median(wordDF$log_freq) # 0.1501654
wordDF$freq_median[wordDF$log_freq>=median_log_freq] <- "H"
wordDF$freq_median[wordDF$log_freq<median_log_freq] <- "L"
mean_log_freq <- mean(wordDF$log_freq) # 0.2017773
wordDF$freq_mean[wordDF$log_freq>=mean_log_freq] <- "H"
wordDF$freq_mean[wordDF$log_freq<mean_log_freq] <- "L"
wordDFmerge <- wordDF[,c("wordform", "log_freq", "freq_median", "freq_mean")]
names(wordDFmerge) <- c("O", "log_freq", "freq_median", "freq_mean")

tr4 <- merge(tr3sa, wordDFmerge, by = c("O"), all.x = TRUE, all.y = TRUE)
write.csv(tr4, "AllRes.csv", row.names=FALSE)


#################### accuracy based on different types of words
tr4 <- read.csv("AllRes.csv")

# strain et al. 1995 case:
strain1995A <- read.csv("Strain-etal-1995-Appendix-A.csv")
word1995A <- merge(wordDFmerge, strain1995A, by = c("O"), all.x=TRUE, all.y=FALSE)
word1995A <- subset(word1995A, freq == "H" | freq == "L")
xtabs(~freq+reg, data=word1995A)
# reg
# freq  E  R
# H 15 14
# L 13 14
unique(word1995A$O[word1995A$freq=="H" & word1995A$reg=="R"])
# [1] best  bill  black bring dark  deal  saw   sense space stay  west  wife  write wrong
word1995A$freq_median[word1995A$freq=="H" & word1995A$reg=="R"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
word1995A$freq_mean[word1995A$freq=="H" & word1995A$reg=="R"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
unique(word1995A$O[word1995A$freq=="H" & word1995A$reg=="E"])
# [1] blood  break  dead   death  does   done   flow   foot   steak  sure   toward want   war   
# [14] watch  worth
word1995A$freq_median[word1995A$freq=="H" & word1995A$reg=="E"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
word1995A$freq_mean[word1995A$freq=="H" & word1995A$reg=="E"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
unique(word1995A$O[word1995A$freq=="L" & word1995A$reg=="R"])
# [1] blade blunt deed  ditch dodge dump  sack  sane  scorn scout weed  wick  wisp  yore 
word1995A$freq_median[word1995A$freq=="L" & word1995A$reg=="R"]
# [1] "H" "H" "H" "H" "L" "H" "H" "L" "L" "H" "H" "L" "L" "L"
word1995A$freq_median[word1995A$freq=="L" & word1995A$reg=="R"]
# [1] "H" "H" "H" "H" "L" "H" "H" "L" "L" "H" "H" "L" "L" "L"
unique(word1995A$O[word1995A$freq=="L" & word1995A$reg=="E"])
# [1] blown  breast debt   dough  dove   dread  scarce suave  swamp  sword  wealth worm   wrath
word1995A$freq_median[word1995A$freq=="L" & word1995A$reg=="E"]
# [1] "H" "H" "H" "H" "H" "H" "H" "L" "H" "H" "H" "H" "L"
word1995A$freq_mean[word1995A$freq=="L" & word1995A$reg=="E"]
# [1] "H" "H" "H" "H" "L" "L" "L" "L" "L" "H" "H" "L" "L"

tr4_1995A <- merge(tr4, strain1995A, by = c("O"), all.x = TRUE, all.y = FALSE)

tr4_freqreg <- subset(tr4_1995A, freq == "H" | freq == "L")
numfreqreg <- length(unique(tr4_freqreg$O)) # 56

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=interaction(freq, reg))) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=interaction(freq, reg))) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_1995.png', dpi = 300, height = 6, width = 12, units = 'in')

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=interaction(freq_mean, reg))) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=interaction(freq_mean, reg))) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_mean.png', dpi = 300, height = 6, width = 12, units = 'in')

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=freq)) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=freq)) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_freq.png', dpi = 300, height = 6, width = 12, units = 'in')

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=reg)) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=reg)) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_reg.png', dpi = 300, height = 6, width = 12, units = 'in')

# Taraban & McClelland 1987 case:
TM1987A <- read.csv("Taraban-McClelland-1987-Appendix-A1.csv", na.strings='na')
word1987A <- merge(wordDFmerge, TM1987A, by = c("O"), all.x=TRUE, all.y=FALSE)
word1987A <- subset(word1987A, freq == "H" | freq == "L")
xtabs(~freq+reg, data=word1987A)
# reg
# freq  E  R
# H 23 24
# L 23 23
unique(word1987A$O[word1987A$freq=="H" & word1987A$reg=="R"])
# [1] best  big   came  class dark  did   fact  got   group him   main  out   page  place
# [15] see   soon  stop  tell  week  when  which will  with  write
word1987A$freq_median[word1987A$freq=="H" & word1987A$reg=="R"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
# [22] "H" "H" "H"
word1987A$freq_mean[word1987A$freq=="H" & word1987A$reg=="R"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
# [22] "H" "H" "H"
unique(word1987A$O[word1987A$freq=="H" & word1987A$reg=="E"])
# [1] are    both   break  choose come   do     does   done   foot   give   great  have  
# [13] move   pull   put    says   shall  want   watch  were   what   word   work  
word1987A$freq_median[word1987A$freq=="H" & word1987A$reg=="E"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
# [22] "H" "H"
word1987A$freq_mean[word1987A$freq=="H" & word1987A$reg=="E"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H"
# [22] "H" "H"
unique(word1987A$O[word1987A$freq=="L" & word1987A$reg=="R"])
# [1] beam  broke bus   deed  float grape lunch peel  pitch pump  ripe  sank  slam  slip 
# [15] stunt swore trunk wake  wax   weld  wing  with  word 
word1987A$freq_median[word1987A$freq=="L" & word1987A$reg=="R"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "L" "H" "H" "H" "H" "L" "H"
# [22] "H" "H"
word1987A$freq_median[word1987A$freq=="L" & word1987A$reg=="R"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "L" "H" "H" "H" "H" "L" "H"
# [22] "H" "H"
unique(word1987A$O[word1987A$freq=="L" & word1987A$reg=="E"])
# [1] bowl  broad bush  deaf  doll  flood gross lose  pear  phase pint  plow  rouse shoe 
# [15] spook swamp swarm touch wad   wand  wash  wool  worm
word1987A$freq_median[word1987A$freq=="L" & word1987A$reg=="E"]
# [1] "H" "H" "H" "H" "H" "H" "H" "H" "H" "H" "L" "L" "L" "H" "L" "H" "L" "H" "L" "L" "H"
# [22] "H" "H"
word1987A$freq_mean[word1987A$freq=="L" & word1987A$reg=="E"]
# [1] "H" "H" "H" "L" "H" "H" "H" "H" "L" "H" "L" "L" "L" "H" "L" "L" "L" "H" "L" "L" "H"
# [22] "H" "L"

tr4_1987A <- merge(tr4, TM1987A, by = c("O"), all.x = TRUE, all.y = FALSE)

tr4_freqreg <- subset(tr4_1987A, freq == "H" | freq == "L")
numfreqreg <- length(unique(tr4_freqreg$O)) # 91

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=interaction(freq, reg))) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=interaction(freq, reg))) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_1987.png', dpi = 300, height = 6, width = 12, units = 'in')

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=interaction(freq_mean, reg))) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=interaction(freq_mean, reg))) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_mean1987.png', dpi = 300, height = 6, width = 12, units = 'in')

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=freq)) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=freq)) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_freq1987.png', dpi = 300, height = 6, width = 12, units = 'in')

ggplot(tr4_freqreg, aes(x=iter, y=accuracy, color=reg)) + scale_x_log10(labels=scinot) + 
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=reg)) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_reg1987.png', dpi = 300, height = 6, width = 12, units = 'in')





strain1995_2002 <- read.csv("Strain1995_2002.csv")
names(strain1995_2002) <- c("O", "reg")
tr5 <- merge(tr3sa, strain1995_2002, by = c("O"), all.x = TRUE, all.y = FALSE)

tr5reg <- subset(tr5, reg == "R" | reg == "E")
numreg <- length(unique(tr5reg$O)) # 124
unique(tr5reg$O)
# [1] best    bill    blade   blotch  blown   bowl    break   breast  bring   brooch  broom  
# [12] carve   choir   chord   chute   clause  cleft   cliff   climb   clink   cloak   comb   
# [23] cough   crude   dark    dead    deaf    deal    death   debt    deed    dense   ditch  
# [34] dose    dread   dreamt  drench  fete    filth   flame   flown   foot    fraud   gait   
# [45] ghost   ghoul   gift    grace   groin   gross   guise   leapt   mauve   moan    mould  
# [56] peck    pint    pour    quart   quest   realm   rude    saint   sane    sauce   scorn  
# [67] scout   seize   sense   shawl   shed    shone   shrewd  sieve   skate   sleight sloth  
# [78] snail   sock    soot    spike   spire   spout   starch  steak   steam   suave   suede  
# [89] suite   sure    swamp   sword   tale    tomb    toward  trend   trough  trout   truce  
# [100] tune    waist   wand    want    war     warn    watch   wealth  web     weed    weep   
# [111] west    whence  wick    wife    wisp    witch   wolf    womb    wool    wreck   wrest  
# [122] write   wrong   yore 
unique(tr5reg$O[tr5reg$reg=="R"])
# [1] best   bill   blade  blotch bring  broom  carve  clause cleft  cliff  clink  cloak  crude 
# [14] dark   deal   deed   dense  ditch  drench filth  flame  fraud  gait   gift   grace  groin 
# [27] moan   peck   quest  rude   saint  sane   sauce  scorn  scout  sense  shawl  shed   shrewd
# [40] skate  snail  sock   spike  spire  spout  starch steam  tale   trend  trout  truce  tune  
# [53] waist  web    weed   weep   west   whence wick   wife   wisp   witch  wreck  wrest  write 
# [66] wrong  yore  
unique(tr5reg$O[tr5reg$reg=="E"])
# [1] blown   bowl    break   breast  brooch  choir   chord   chute   climb   comb    cough  
# [12] dead    deaf    death   debt    dose    dread   dreamt  fete    flown   foot    ghost  
# [23] ghoul   gross   guise   leapt   mauve   mould   pint    pour    quart   realm   seize  
# [34] shone   sieve   sleight sloth   soot    steak   suave   suede   suite   sure    swamp  
# [45] sword   tomb    toward  trough  wand    want    war     warn    watch   wealth  wolf   
# [56] womb    wool 

ggplot(tr5reg, aes(x=iter, y=accuracy, group=reg, color=reg)) + scale_x_log10(labels=scinot) +
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=reg)) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuReg_1995_2002.png', dpi = 300, height = 6, width = 12, units = 'in')


## frequency effect based on own data
wordlist <- read.csv("extwords3_Harm1998.csv")
median_log_freq <- median(wordlist$log_freq)
wordlist$freq[wordlist$log_freq>=median_log_freq] <- "H"
wordlist$freq[wordlist$log_freq<median_log_freq] <- "L"
wordformerge <- wordlist[,c("wordform", "freq")]
names(wordformerge) <- c("O", "freq")

tr6 <- merge(tr3sa, wordformerge, by = c("O"), all.x = TRUE, all.y = FALSE)
tr6freq <- subset(tr6, freq == "H" | freq == "L")
ggplot(tr6freq, aes(x=iter, y=accuracy, group=freq, color=freq)) + scale_x_log10(labels=scinot) +
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=freq)) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreq_own.png', dpi = 300, height = 6, width = 12, units = 'in')

tr7 <- merge(tr5, wordformerge, by = c("O"), all.x = TRUE, all.y = FALSE)
tr7freqreg <- subset(tr7, reg == "R" | reg == "E")
ggplot(tr7freqreg, aes(x=iter, y=accuracy, color=interaction(freq, reg))) + scale_x_log10(labels=scinot) +
  #geom_line(aes(colour=fr, group=fr)) +
  coord_cartesian(xlim = c(1e4, 1e8)) +
  xlab("Training Trials (log10)") + ylab("Avg Accuracy") +  
  ggtitle("Avg Accuracy x Training Trials\n Combo of Hidden Layer Size & Learning Rate") +
  #geom_point(alpha=.2, aes(color=reg)) + 
  geom_smooth(span=.2, aes(color=interaction(freq, reg))) +
  facet_grid(lrnrate~hlsize)
ggsave('AcuFreqReg_1995_2002.png', dpi = 300, height = 6, width = 12, units = 'in')
