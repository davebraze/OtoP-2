require(ggplot2)
require(plyr)
require(reshape2)
require(sjPlot)
require(Hmisc)
require(GGally)
require(stats)
require(xlsx)
#require(moments)
require(forecast)
require(lme4)
require(tm)
require(MASS)
require(e1071)
require(car)

# prepare all the data in different runs
# collect results of all runs;
numRun <- 5

# for output file
outFile <- "output.txt"
FileLoc <- paste("./1/", outFile, sep="")
Dout <- read.table(FileLoc, header = TRUE, sep = "\t", quote = "\"'", dec = ".")
Dout$runID <- 1
for(i in 1:numRun){
  FileLoc <- paste("./", i, "/", outFile, sep="")
  Dtemp <- read.table(FileLoc, header = TRUE, sep = "\t", quote = "\"'", dec = ".")
  Dtemp$runID <- i
  Dout <- rbind(Dout, Dtemp)
}
Dout <- Dout[,c(16,13,14,15,1:12)]
write.csv(Dout, "./outputAll.csv", row.names = FALSE)

# for itemacu_tr
acuTRFile <- "itemacu_tr.txt"
FileLoc <- paste("./1/", acuTRFile, sep="")
Dacu_tr <- read.table(FileLoc, header = TRUE, sep = "\t", quote = "\"'", dec = ".")
Dacu_tr$runID <- 1
for(i in 1:numRun){
  FileLoc <- paste("./", i, "/", acuTRFile, sep="")
  Dtemp <- read.table(FileLoc, header = TRUE, sep = "\t", quote = "\"'", dec = ".")
  Dtemp$runID <- i
  Dacu_tr <- rbind(Dacu_tr, Dtemp)
}
Dacu_tr <- Dacu_tr[,c(308,1,307,2:306)]
write.csv(Dacu_tr, "./acu_trAll.csv", row.names = FALSE)

# for itemacu_te
acuTEFile <- "itemacu_te.txt"
FileLoc <- paste("./1/", acuTEFile, sep="")
Dacu_te <- read.table(FileLoc, header = TRUE, sep = "\t", quote = "\"'", dec = ".")
Dacu_te$runID <- 1
for(i in 1:numRun){
  FileLoc <- paste("./", i, "/", acuTEFile, sep="")
  Dtemp <- read.table(FileLoc, header = TRUE, sep = "\t", quote = "\"'", dec = ".")
  Dtemp$runID <- i
  Dacu_te <- rbind(Dacu_te, Dtemp)
}
Dacu_te <- Dacu_te[,c(308,1,307,2:306)]
write.csv(Dacu_te, "./acu_teAll.csv", row.names = FALSE)
