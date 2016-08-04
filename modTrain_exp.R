library(stringr)

te <- readLines("training_examples.txt")

prob <- str_extract(te,  "FOO: ([0-9.]+)")
prob <- str_replace(prob, "FOO: ", "PROB ")
loc <- which(!is.na(prob))

head(te, 50)

for (ll in rev(loc)) {
    print(ll)
    te <- append(te, prob[ll], after=ll)
}

head(te, 50)

## write out the new file
fileConn <- file("newtraining_examples.txt")
writeLines(te, fileConn)
close(fileConn)
