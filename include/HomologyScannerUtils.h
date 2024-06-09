std::string chainIdFromPrimaryComplex ; // One chain ID in primary complex to which contents of struct are related

struct HomologousPdbIdChainAndSequenceIdentity{ // One such struct contains all info relevant to one homologous chain
    std::string pdbId ;
    std::string chainId ;
    std::string sequence ;
    double eValue;
    double sequenceIdentity ; // in %.
 }

struct ChainIdAndComplexNumber {
    std::string chainId;
    int complexNumber; // This will usually be 0 or 1.  Should probably raise a flag if it is anything else.
    std::string sequence;
}

struct ChainIdAndHomologData {
    ChainIdAndHomologData(): primaryChainIdAndComplexNumber.chainId(""), primaryChainIdAndComplexNumber.complexNumber(0) { }    // default Constructor
homologousPdbIdChainsAndSequenceIdentities(,y(0), z(0), w(0) { }   // default Constructor
    ChainIdAndComplexNumber primaryChainIdAndComplexNumber ; // chain ID in primary complex to which the below homologousPdbIdChainsAndSequenceIdentities are related
    bool homologousPdbIdChainsAndSequenceIdentitiesHaveBeenSet;
    vector <HomologousPdbIdChainAndSequenceIdentity> homologousPdbIdChainsAndSequenceIdentities(0); // I believe this initializes the vector to zero length. This is the fill constructor, without second parameter
}

struct HomologComplex {
    std::string pdbId;
    vector <ChainIdAndComplexNumber> chainsAndComplexNumbers;
}h
