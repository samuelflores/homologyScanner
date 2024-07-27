#include "PrimaryJobData.h"
#include "ParameterReader.h"
#include "Repel.h"
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h> // Required by access()
#include <seqan/align.h>
#include <iomanip> // needed for 	to_string_with_precision
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <thread>
#include <curl/curl.h>
//#include <experimental/filesystem>
//# #include <sys/stat.h>

#define PARENTCHAINPREFIX "P"
#define SEQUENCEIDENTITYCUTOFF 90.0

int GenericJob::checkFileStatus(std::string myFileName){/*
    ifstream testFile(myFileName.c_str());
    int returnValue = (testFile.good()); // Returns true for good file, false for bad file.
    testFile.seekg(0, ios::end);
    int file_size = testFile.tellg();
    testFile.close();
    if (returnValue){ // case file is bad or does not exist:
        std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Owner does NOT appear to have read  permissions on  file >"<<myFileName <<"<. Please confirm that the file exists and is accessible."<<std::endl;
        //exit(1);

    } else {
        std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Owner DOES appear to have read  permissions on  file >"<<myFileName <<std::endl;
    }
    if (file_size == 0) {
        returnValue = 0;
        std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The file being checked has zero size. Removing it. Returning "<<returnValue<<std::endl;
        system(std::string("rm " + myFileName).c_str());
    }
    */
    //return returnValue;
    return dbConnection->checkFileStatus(myFileName);
}
// myMkdir is now in MMB's include/Utils.h
/*
int myMkdir(std::string directoryPath){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" You are asking to create the directory  "<<directoryPath<<std::endl;
    if (!(opendir(directoryPath.c_str()))){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" opendir failed to open directory "<<directoryPath<<" . Will now create this directory.  " <<std::endl;
        const int dir_err = mkdir(directoryPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<": mkdir returned : "<<dir_err <<std::endl;
        if (-1 == dir_err)
        {
            printf("Error creating directory!n");
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Failed to  create directory "<<directoryPath<<"  " <<std::endl;
            exit(1);
        } else if (0 == dir_err) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Successfully created directory "<<directoryPath<<" " <<std::endl;
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" An unexpected error occurred when creating the directory "<<directoryPath<<" " <<std::endl;
        }
    }
    if (access((directoryPath ).c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a directory called "<< directoryPath   <<" . So far so good."<<std::endl;
        return 0;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! Found that we do NOT have read access to a directory called "<<  directoryPath  <<"  "<<std::endl; exit(1);
        return 1;
    }   
}*/

int myChdir(std::string directoryPath){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to attempt changing directory to "<<directoryPath<<" . "<<std::endl;
    if (chdir(directoryPath.c_str())){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Unable to change directory to "<<directoryPath<<" . Exiting now."<<std::endl;
        exit(1);
        return 1;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Was able to successfully change directory to "<<directoryPath<<" . "<<std::endl;
        return 0;
    }
}

std::string GenericJob::getPdbId(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Obsolete!"<<std::endl;   
    exit(1); /*
    std::string myPdbId = dbConnection->getPdbId();
    if (myPdbId.length() != 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! Length of PDB ID is "<<myPdbId.length()<<" . Expected 4!"<<std::endl; exit(1);
    }
    return myPdbId;*/
}

std::string PrimaryJob::getPdbId(){
    std::string myPdbId = dbConnection->getPdbId();
    if (myPdbId.length() != 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! Length of PDB ID is "<<myPdbId.length()<<" . Expected 4!"<<std::endl; exit(1);
    }
    return myPdbId;
}


std::string HomologJob::getPdbId(){
    std::string myPdbId  = getCachedPdbId(); // dbConnection->getPdbId();
    if (myPdbId.length() != 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  Length of cached PDB ID "<<myPdbId<<" is "<<myPdbId.length()<<" . Expected 4! Using MySQL's PDB ID instead.."<<std::endl;  //exit(1);
        myPdbId = dbConnection->getPdbId();//getPdbId();
    }
    if (myPdbId.length() != 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! Length of MySQL's PDB ID "<<myPdbId<<" is now "<<myPdbId.length()<<" . Expected 4! Cannot recover from this error."<<std::endl; exit(1);
    }
    // not sure if we have a dbConnection open, but would be good to check:
    /*if (getCachedPdbId() != dbConnection->getPdbId()) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! MySQL's PDB ID "<< dbConnection->getPdbId()  <<std::endl;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Does not match cached PDB ID :  "<< getCachedPdbId()   <<" . First set cachedPdbId, then set MySQL's PDB ID."<<std::endl; exit(1);
    } */   
  
    return myPdbId;
}

void HomologJob::copyChainsFromParent()            {
   std::map <std::string, int> myParentChainAndComplexNumberMap; // the key is the chain ID of the primary job  (which spawned us), the mapped value is the chain ID of the current homolog job.
   //myParentChainAndComplexNumberMap.clear();
                primaryToHomologChainIdMap.clear();
   myParentChainAndComplexNumberMap = getParentPrimaryJobPointer()->getChainAndComplexNumberMap();
   for (auto it=myParentChainAndComplexNumberMap.begin(); it!=myParentChainAndComplexNumberMap.end(); ++it){
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " inserting into primaryToHomologChainIdMap : >"<< it->first <<"< , >" << it->first <<"< "<<std::endl;
       primaryToHomologChainIdMap.insert(std::pair<std::string , std::string> (it->first, it->first));
   }
   std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " getParentPrimaryJobPointer()->setHasBeenCloned(1) "<<std::endl;
   getParentPrimaryJobPointer()->setHasBeenCloned(1);
   validate(); // Among other things, this checks that primaryToHomologChainIdMap and etParentPrimaryJobPointer()->getChainAndComplexNumberMap() have the same size
   //exit (1); 
}

std::string HomologJob::getCorrespondingChain(std::string chainInPrimaryJob) {
   std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " taking parent (primary) chain >"<< chainInPrimaryJob  << "< "<<std::endl;
   auto it = primaryToHomologChainIdMap.find(chainInPrimaryJob);
   if (it == primaryToHomologChainIdMap.end()){std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! "<<std::endl;  exit(1);}
   else { 
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Returning chain >"<< it->second << "< "<<std::endl;
       return it->second;}
}

void HomologJob::setCorrespondingChain(std::string chainInPrimaryJob, std::string chainInHomologJob) {
    auto it = primaryToHomologChainIdMap.find(chainInPrimaryJob);
   if (it == primaryToHomologChainIdMap.end()){
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Could not find primary job chain "<< chainInPrimaryJob <<" mapping in the current HomologJob. Will now define this chain to map to homolog job chain "<< chainInHomologJob<< " in current HomologJob. "<<std::endl;
       primaryToHomologChainIdMap.insert(std::pair<std::string, std::string> (chainInPrimaryJob,chainInHomologJob));
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" You are trying to map primary job chain "<< chainInPrimaryJob <<" to homolog job chain "<< chainInHomologJob<< " . However a  primary job chain "<<  it->first << " exists which maps to homolog job chain "<<  it->second << " . For now we are treating this condition as an error. "<<std::endl;
        exit(1);
    }
}


std::string HomologJob::getRenumberedPdbFileName(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // If there is no row at all, this will create one:
    std::string tempRenumberedPdbFileName = (*getParentPrimaryJobPointer()).breederParameterReader.breederMainDirectory + "/renumberedPdbFiles/" +getPdbId()+".renumbered.pdb";	  
    dbConnection->insertIntoAlternatePdbStructuresIfAbsent("alternateRenumberedStructureFile", tempRenumberedPdbFileName) ; // This should  also set status = SUCCESS
    // We need to be super careful to get the same one that breeder would be getting. To that end, also super careful that we are using the HomologJob's (ratehr than the parent's) PDB ID. 
    // If a row existed above, the aprticular column alternateRenumberedStructureFile may still be empty:
    std::string myRenumberedPdbFileName = dbConnection-> getFromAlternatePdbStructures("alternateRenumberedStructureFile",getPdbId());  // This will fetch the renumbered file name from  table alternatePdbStructures. We need to trap the error that this has not been set, or that the file does not exist.
    if (myRenumberedPdbFileName == "") {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" If you have not set alternateRenumberedStructeFile, will do so now .  "<<std::endl;
        dbConnection->updateOrInsertInAlternatePdbStructures("alternateRenumberedStructureFile", tempRenumberedPdbFileName, "SUCCESS"); // since we earlier called insertIntoAlternatePdbStructuresIfAbsent, this method can here logically only act in Update mode. 
        //exit(1);
    }
    myRenumberedPdbFileName = dbConnection-> getFromAlternatePdbStructures("alternateRenumberedStructureFile",getPdbId());  // Now there should be no reason for this to be an empty string. The file with this name, however, might not exist .. this might be ok for now.
    if (myRenumberedPdbFileName == "") {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Somehow you wound up with an empty string in alternateRenumberedStructureFile .. no idea how this oculd have happened."<<std::endl;
        exit(1);
    }
    if (access(myRenumberedPdbFileName.c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a structure file called "<<         myRenumberedPdbFileName <<" . Returning that structure file name."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! We found that we do NOT have read access to a structure file called "<<         myRenumberedPdbFileName <<" . This may be OK, if you are planning to do it later.. "<<std::endl;
        //exit(1); 
    }   
    return myRenumberedPdbFileName;
    //return (*getParentPrimaryJobPointer()).breederParameterReader.breederMainDirectory + "/renumberedPdbFiles/" +getPdbId()+".renumbered.pdb";
}

int HomologJob::writeRenumberedPdbIfAbsent(){

    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Checking for existence of getRenumberedPdbFileName() = " << getRenumberedPdbFileName().c_str() << std::endl; 

    struct stat st;
    if(stat(getRenumberedPdbFileName().c_str(), &st) != 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" stat says no file exists. So we do need to create this file. Let's continue. "<<std::endl; 
        //return 0;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" stat says the file exists. So probably there is no need to create this file. Let's check the file size to be sure.. "<<std::endl; 
        if ( st.st_size == 0){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" However apparently "<<getRenumberedPdbFileName()<<" has zero size. Please delete this file and try again."<<std::endl; 
            exit(1);
            return 1;
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The file "<<getRenumberedPdbFileName()<<" has size "<<st.st_size <<". So no need to create this file. Let's continue.."<<std::endl; 
            //dbConnection->updateOrInsertAlternateStructureFileName(getRenumberedPdbFileName(),"SUCCESS"); // breeder will get the file name to work with from table alternatePdbStructures .
            //dbConnection->setPdbStatusSuccess(); // Not sure if we should wait even longer to declare success. here all we are saying is that the coordinate matching worked, which is the usual problem. // update: decided we do need to wait to declare success
            return 0;
        }
    }
    // If we did not trip the above block 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    ParameterReader myTempParameterReader;// = new ParameterReader();
    
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " myTempParameterReader.leontisWesthofInFileName = "<<myTempParameterReader.leontisWesthofInFileName <<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " std::string(getParentPrimaryJobPointer()->breederParameterReader.singleMutantFilesDirectory + /parameters.csv).c_str()) = "<<std::string(getParentPrimaryJobPointer()->breederParameterReader.singleMutantFilesDirectory + "/parameters.csv").c_str()<<std::endl; 
    myTempParameterReader.initializeDefaults(std::string(getParentPrimaryJobPointer()->breederParameterReader.singleMutantFilesDirectory + "/parameters.csv").c_str());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " myTempParameterReader.leontisWesthofInFileName = "<<myTempParameterReader.leontisWesthofInFileName <<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    myTempParameterReader.myBiopolymerClassContainer.loadSequencesFromPdb(chooseStructureFileName(),false, "",false,true);
    //// Now populate the sequence table
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    // No idea why this was commented out before. If this works, remove the corresponding from main.cpp. But it does not wor
    // getMutationString() is called on this, which is a HomologJob
    // However here getMutationString() returns a zero length string! must not have been set yet..
    // translateMutationVectorFromParent does this .. around line 1265. moving this there.
    //Chromosome tempChromosome(  getMutationString(), myTempParameterReader.myBiopolymerClassContainer );
    //tempChromosome.populateSequenceTable(*dbConnection);
    //// Sequence table should be populated now with both PDB and renumbered residue numbers
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    myTempParameterReader.myBiopolymerClassContainer.setRenumberPdbResidues(1);  
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    ConstrainedDynamics  myTempConstrainedDynamics(&myTempParameterReader);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    myTempConstrainedDynamics.initializeDumm();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    myTempParameterReader.previousFrameFileName = chooseStructureFileName();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    myTempConstrainedDynamics.initializeBiopolymersAndCustomMolecules();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Starting to write >"<<getRenumberedPdbFileName()<<"< "<<std::endl; 
    std::ofstream tempStream(String(getRenumberedPdbFileName()).c_str(),ios_base::out);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    myTempParameterReader.myBiopolymerClassContainer.writeDefaultPdb(  tempStream); 
    tempStream.close();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    //dbConnection->updateOrInsertAlternateStructureFileName(getRenumberedPdbFileName(),"SUCCESS"); // breeder will get the file name to work with from table alternatePdbStructures .
    /**/
    return 0; // returns 0 if all is OK
}



void  HomologJob::setPdbId(std::string myPdbId){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" this function is being called with argument : >"<<myPdbId<<"<"<<std::endl;
    if (myPdbId.length() != 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  Length of provided  PDB ID "<<myPdbId<<" is "<<myPdbId.length()<<" . Expected 4! "<<std::endl;  exit(1);} 
    else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  Length of provided  PDB ID "<<myPdbId<<" is "<<myPdbId.length()<<" . Which is the Expected 4. "<<std::endl;  } 
    
    if (myPdbId != getCachedPdbId()) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The  provided  PDB ID "<<myPdbId<<" does not match the cached one : "<< getCachedPdbId()<<std::endl;  exit(1);
    }
    dbConnection->setPdbId(myPdbId);
    //dbConnection->setJobID(myPdbId);
}



std::string  GenericJob::getDefaultPdbNumberedStructureFileName(){   //(std::string myStructureFileName){
    // This is the default file name in case no such file has been manually prepared. Should think of some way to prevent the manually prepared version is never overwritten. getDefaultPdbNumberedStructureFileName() - indicated file CAN be overwritten with the wget command below.
    /*
    std::string defaultStructureFileName = getWorkingDirectory() + getPdbId() + std::string(".pdb");
    // should check that the  getWorkingDirectory()   exists and is readable and writeable. Do NOT check the full defaultStructureFileName, because it may not have been writen yet.
    if (access(getWorkingDirectory().c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a directory called "<< getWorkingDirectory()   <<" . So far so good."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! Found that we do NOT have read access to a directory called "<< getWorkingDirectory()   <<" . Maybe you have not yet set this up. That may be OK."<<std::endl; //exit(1);
    }   
    if (access(getWorkingDirectory().c_str(), W_OK) == 0) {
    //if (access(getWorkingDirectory().c_str(), W_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have write access to a directory called "<< getWorkingDirectory()   <<" . So far so good."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! Found that we do NOT have write access to a directory called "<< getWorkingDirectory()   <<" . Maybe you have not yet set this up. That may be OK. "<<std::endl; //exit(1);
    }*/
    //return defaultStructureFileName;   
    return dbConnection->getDefaultPdbNumberedStructureFileName();
}

/*
void  GenericJob::setStructureFileName(){   //(std::string myStructureFileName){
    // should check that file exists and is readable.
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Obsolete!"<<std::endl;   
    exit(1);
    structureFileName = getWorkingDirectory() + getPdbId() + std::string(".pdb");//myStructureFileName;
}*/

std::string  GenericJob::getPdbNumberedStructureFileName(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to call dbConnection->getAlternateStructureFileName() .."<<std::endl;   
    //return dbConnection->getWildTypeStructureFileName ();     // This function does everything .. checks for an alternate structure file name, constructs a default one otherwise.
    //return dbConnection->getAlternateStructureFileName(); // This function ONLY fetches an alternateStructureFileName from the mysql database. The more common case is that of anempty string here , "".                          
    //std::string structureFileName =  dbConnection->getAlternateStructureFileName(); // This function ONLY fetches an alternateStructureFileName from the mysql database. The more common case is that of anempty string here , "".                          
    // should check that file exists and is readable.
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;    
    printData();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;    
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<dbConnection->getPdbId()<<endl;
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<dbConnection->getJobID()<<endl;
    std::string structureFileName;   
    // Sometimes we specify a manually edited file in alternatePdbStructures.alternateStructureFile . First check if that is the case:
    if (dbConnection->getPdbNumberedWildTypeStructureFileName() != std::string("")) {
        structureFileName = dbConnection->getPdbNumberedWildTypeStructureFileName();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" dbConnection->getPdbNumberedWildTypeStructureFileName() returned : "<<structureFileName<<std::endl;
    } 
    else { // Otherwise, construct a default file name :
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Something is wrong if we are here. This should have been composed in getPdbNumberedWildTypeStructureFileName ."<<std::endl;
        exit(1);
        //structureFileName = getDefaultPdbNumberedStructureFileName();//getWorkingDirectory() + getPdbId() + std::string(".pdb");
        //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" getDefaultPdbNumberedStructureFileName() returned : "<<structureFileName<<std::endl;
    }
    if (access(structureFileName.c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a structure file called >"<<         structureFileName <<"< . Returning that structure file name."<<std::endl;
        return structureFileName;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! We found that we do NOT have read access to a structure file called "<<         structureFileName <<" . It could be that you simply have not yet downloaded it. "<<std::endl;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Something is wrong if we are here. This should have been fetched in getPdbNumberedWildTypeStructureFileName ."<<std::endl;
        exit(1); 
    }   
    
}


std::string  HomologJob::getAlignedStructureFileName(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
    std::string myAlignedStructureFileName = getWorkingDirectory() + "/last.align-homolog-on-primary.pdb";
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Aligned structure file name should be "<<myAlignedStructureFileName<<std::endl;
    return myAlignedStructureFileName;
}

// This function returns getAlignedStructureFileName() if that is available, getPdbNumberedStructureFileName() otherwise. also checks the latter for read permissions
    std::string  HomologJob::chooseStructureFileName(){
    std::string myAlignedStructureFileName = getAlignedStructureFileName();
    std::string myStructureFileName = getPdbNumberedStructureFileName();
    if (access(myAlignedStructureFileName.c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to an aligned structure file called "<<myAlignedStructureFileName <<" . So we will load that instead of the original file downloaded from the PDB,"<<getPdbNumberedStructureFileName()<<std::endl;
        return getAlignedStructureFileName();
    } else if (access(myStructureFileName.c_str(), R_OK) == 0) {
        //  check that file exists and is readable.
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we  do NOT have read access to a  structure file called "<<myAlignedStructureFileName  <<" . However we do have the original PDB file, "<<myStructureFileName<<" , so we will use that."<<std::endl; 
        return myStructureFileName;
    } else {
       //dbConnection->setPdbStatusFail();   // Set status to FAIL so we do not try to fetch this one again.                                                                    
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we  do NOT have read access to a  structure file called "<<myStructureFileName  <<" . This is a problem! "<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__ <<std::endl; exit(1);
    }
}

bool HomologJob::calcLocalSequenceAlignment(vector<ThreadingStruct> threadingStructVector, State state){ // passing by value, not by reference.This is important! Do not wish to destroy the original.
    int match = 0; int misMatch = 0;
    BiopolymerClassContainer homologBiopolymerClassContainerCopy = updBiopolymerClassContainer();
    BiopolymerClassContainer primaryBiopolymerClassContainerCopy = (*getParentPrimaryJobPointer()).updBiopolymerClassContainer();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
    double myRadius = 0.3; // in nm. This is the distance about the mutation site which will be included in the local sequence alignment.
    vector <AllResiduesWithin> homologAllResiduesWithinVector; homologAllResiduesWithinVector.clear();
    for (int i = 0; i < homologBiopolymerClassContainerCopy.updMutationVector().size() ; i++) {
        homologAllResiduesWithinVector.push_back(homologBiopolymerClassContainerCopy.updMutationVector()[i].allResiduesWithin(myRadius));}  
    vector <AllResiduesWithin> primaryAllResiduesWithinVector; primaryAllResiduesWithinVector.clear();
    for (int i = 0; i < primaryBiopolymerClassContainerCopy.updMutationVector().size() ; i++) {
        primaryAllResiduesWithinVector.push_back(primaryBiopolymerClassContainerCopy.updMutationVector()[i].allResiduesWithin(myRadius));}  

    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
    vector<SingleResidue> includedSingleResidueVectorInHomologJob; includedSingleResidueVectorInHomologJob.clear();
    includedSingleResidueVectorInHomologJob = homologBiopolymerClassContainerCopy.findBiopolymerResiduesWithinRadius(homologAllResiduesWithinVector ,  state);
    vector<SingleResidue> includedSingleResidueVectorInPrimaryJob; includedSingleResidueVectorInPrimaryJob.clear();
    includedSingleResidueVectorInPrimaryJob = primaryBiopolymerClassContainerCopy.findBiopolymerResiduesWithinRadius(primaryAllResiduesWithinVector , state);

  
    for (int i = 0; i < threadingStructVector.size() ; i ++){
        // Per convention partner 0 is the homologJob, partner 1 is the PrimaryJob
        // Actually we need to filter by chain ID:
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
        threadingStructVector[i].updThreadingPartner(0).includedResidues = threadingStructVector[i].updThreadingPartner(0).biopolymerClass.filterSingleResidueVector(includedSingleResidueVectorInHomologJob);
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
        threadingStructVector[i].updThreadingPartner(1).includedResidues = threadingStructVector[i].updThreadingPartner(1).biopolymerClass.filterSingleResidueVector(includedSingleResidueVectorInPrimaryJob);
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
        // recall that filterSingleResidueVector also conveniently sorts the vector for us.  
        threadingStructVector[i].supplementIncludedResidues(); // This takes any residues in bipolymerClass 0 that align with a residue in bipolymerClass 1, and makes sure the latter residue is present in the includedResidues for biopolymerClass 1.  It also does the converse.
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
        threadingStructVector[i].setShortSequences(); // sets the "sequence" fields to the sequence corresponding to includedResidues. Recall these are sorted, so should work. This also calls computeAlign(), because we don't want to risk align being out of sync.
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
        threadingStructVector[i].printAlignmentStats(); 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;

        // Per convention partner 0 is the homologJob, partner 1 is the PrimaryJob
        dbConnection->setLocalSequenceIdentity( (*getParentPrimaryJobPointer()).getPdbId()  ,  getPdbId(), 
            threadingStructVector[i].updThreadingPartner(1).biopolymerClass.getChainID(),  
            threadingStructVector[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            (*getParentPrimaryJobPointer()).getMutationString(),
            myRadius, 
            std::string("matches"), threadingStructVector[i].getAlignmentStats().numMatches); 
        dbConnection->setLocalSequenceIdentity( (*getParentPrimaryJobPointer()).getPdbId()  ,  getPdbId(), 
            threadingStructVector[i].updThreadingPartner(1).biopolymerClass.getChainID(),  
            threadingStructVector[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            (*getParentPrimaryJobPointer()).getMutationString(),
            myRadius, 
            std::string("mismatches"), threadingStructVector[i].getAlignmentStats().numMismatches); 
        dbConnection->setLocalSequenceIdentity( (*getParentPrimaryJobPointer()).getPdbId()  ,  getPdbId(), 
            threadingStructVector[i].updThreadingPartner(1).biopolymerClass.getChainID(),  
            threadingStructVector[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            (*getParentPrimaryJobPointer()).getMutationString(),
            myRadius, 
            std::string("insertions"), threadingStructVector[i].getAlignmentStats().numInsertions); 
        dbConnection->setLocalSequenceIdentity( (*getParentPrimaryJobPointer()).getPdbId()  ,  getPdbId(), 
            threadingStructVector[i].updThreadingPartner(1).biopolymerClass.getChainID(),  
            threadingStructVector[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            (*getParentPrimaryJobPointer()).getMutationString(),
            myRadius, 
            std::string("deletions"), threadingStructVector[i].getAlignmentStats().numDeletions); 

        // Use this to set pass fail criterion:
        match += threadingStructVector[i].getAlignmentStats().numMatches;
        misMatch += threadingStructVector[i].getAlignmentStats().numMismatches;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<std::endl;
    if ((match / (match + misMatch)) > .9) return 0 ; else return 1; // Here zero is success, 1 is fail.s
}

void  GenericJob::validate(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" The PDB ID of this job is: >"<< getPdbId()<<"< . This is "<<getPdbId().length()<<" characters long."<<std::endl;
    if (getPdbId().length() == 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" The length of "<<getPdbId().length()<<" is OK. "<<std::endl; 
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" The length of "<<getPdbId().length()<<" indicates a problem. Exiting now. "<<std::endl; exit(1);}
} // of validate

void  HomologJob::validate(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" The PDB ID of this job is: >"<< getPdbId()<<"< . This is "<<getPdbId().length()<<" characters long."<<std::endl;
    if (getPdbId().length() == 4){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" The length of "<<getPdbId().length()<<" is OK. "<<std::endl; 
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" The length of "<<getPdbId().length()<<" indicates a problem. Exiting now. "<<std::endl; exit(1);}

    if (primaryToHomologChainIdMap.size() != getParentPrimaryJobPointer()->getChainAndComplexNumberMap().size()){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Size of primaryToHomologChainIdMap : " << primaryToHomologChainIdMap.size() << " does not match that of the parent PrimaryJob's chainAndComplexNumberMap : "<< getParentPrimaryJobPointer()->getChainAndComplexNumberMap().size()<<std::endl; exit(1);
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Size of primaryToHomologChainIdMap : " << primaryToHomologChainIdMap.size() << "  matches that of the parent PrimaryJob's chainAndComplexNumberMap : "<< getParentPrimaryJobPointer()->getChainAndComplexNumberMap().size()<<std::endl; 
    }


} // of validate
/*
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    string data((const char*) ptr, (size_t) size * nmemb);
    *((stringstream*) stream) << data << endl;
    return size * nmemb;
}*/

int pullFileWithCurl(std::string urlOfOriginFile, std::string destinationFileNameWithPath)
{
    // Adapted from Uli Köhler (techoverflow.net), contribution to public domain .
    //std::string url = std::string("http://www.rcsb.org/pdb/files/") + pdbId + std::string(".pdb");  
    CURL *curl = curl_easy_init();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to call curl_easy_setopt(curl, CURLOPT_URL, "<<urlOfOriginFile<<");"<<std::endl;
    curl_easy_setopt(curl, CURLOPT_URL, urlOfOriginFile.c_str());
    /// could be redirected, so we tell libcurl to follow redirection 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    FILE * outFile;

    //////
    // To potentially save a bit of time, let's see if maybe this file already exists:
    //////
    std::ifstream testFileStream(destinationFileNameWithPath);
    if ( testFileStream.good() ) { // There was a ! before this, which I have removed 18 April 2019
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a file called "<<destinationFileNameWithPath<<" . So no need to fetch this file from the PDB. Returning from function now."<<std::endl;
        testFileStream.close();
        return 0;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we do NOT have read access to a file called "<<destinationFileNameWithPath<<" . So we DO need to fetch this file from the PDB. Let's continue. "<<std::endl;
    }
    testFileStream.close();
    //////
 
    outFile = fopen (destinationFileNameWithPath.c_str(),"w");
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    //  CURLOPT_WRITEFUNCTION defaults to fwrite
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outFile);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // Perform the request, res will get the return code 
    CURLcode res = curl_easy_perform(curl);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // Check for errors   
    if (res != CURLE_OK) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    fclose(outFile);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done with "<<destinationFileNameWithPath<<std::endl;
    return 0;
}
void filecopy(FILE *dest, FILE *src)
{
    const int size = 16384;
    char buffer[size];

    while (!feof(src))
    {
        int n = fread(buffer, 1, size, src);
        fwrite(buffer, 1, n, dest);
    }

    fflush(dest);
}
void fileopen_and_copy(const char * dest, const char * src)
{
    FILE * infile  = fopen(src,  "rb");
    FILE * outfile = fopen(dest, "wb");

    filecopy(outfile, infile);

    fclose(infile);
    fclose(outfile);
}

void GenericJob::fetchPdb(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" getWorkingDirectory() = >"<<getWorkingDirectory()<<"< "<<std::endl; 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" dbConnection->getWorkingDirectory() = >"<<dbConnection->getWorkingDirectory()<<"< "<<std::endl; 
    dbConnection->getPdbNumberedWildTypeStructureFileName (); // this gets the file name from the alternatePdbStructures table, otherwise composes one and if needed fetches the PDB file from RCSB. The old fetchPdb() was much longer, but this command is really all you need now.
}
/*
{
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" getWorkingDirectory() = >"<<getWorkingDirectory()<<"< "<<std::endl; 
    if (getPdbId().length() == 0){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The PDB ID : "<< getPdbId() << " has length "<< getPdbId().length() <<" . We are treating this as an error. "<<std::endl; exit (1);
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Fetching file for  PDB ID : >"<< getPdbId()<<"<" <<std::endl;
    // by issuing this, you are saying that you want to set structureFileName as follows:
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" will be using structure   : "<< getPdbNumberedStructureFileName() <<std::endl; // This returned either a default or an alternate structure file name.


    if (!(opendir(getWorkingDirectory().c_str()))){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" opendir failed to open directory "<<getWorkingDirectory()<<" . Will now create this directory.  " <<std::endl;
        const int dir_err = mkdir(getWorkingDirectory().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (-1 == dir_err)
        {
            printf("Error creating directory!n");
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Failed to  create directory "<<getWorkingDirectory()<<"  " <<std::endl;
            exit(1);
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Successfully created directory "<<getWorkingDirectory()<<" " <<std::endl;
        }
    }
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" About to issue :"<<std::endl<<systemCall2<<std::endl;
    //system(systemCall2.c_str());

    if (access(getWorkingDirectory().c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a directory called "<< getWorkingDirectory()   <<" . So far so good."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! Found that we do NOT have read access to a directory called "<< getWorkingDirectory()   <<"  "<<std::endl; exit(1);
    }   
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // getDefaultPdbNumberedStructureFileName() has the advantage that it also tests the directory the file will sit in, to make sure it is accessible.
    std::string myDestinationFileNameWithPath = getDefaultPdbNumberedStructureFileName();// getWorkingDirectory() + std::string("/") + getPdbId() + std::string(".pdb");
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;

    //std::string myDefaultStructureFileName   = getDefaultPdbNumberedStructureFileName();
    std::string myAlternateStructureFileName = dbConnection->getPdbNumberedWildTypeStructureFileName();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    if (myAlternateStructureFileName == getPdbNumberedStructureFileName()){
         if (access(myAlternateStructureFileName.c_str(), R_OK) == 0) {
             std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Copying "<<myAlternateStructureFileName<<" to "<<myDestinationFileNameWithPath<<std::endl;
             fileopen_and_copy( myDestinationFileNameWithPath.c_str(), myAlternateStructureFileName.c_str());
             //fileopen_and_copy(myAlternateStructureFileName.c_str(), myDestinationFileNameWithPath.c_str());
             //std::filesystem::copy(myAlternateStructureFileName, myDestinationFileNameWithPath)
         }
         else {
             std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Unable to access "<<myAlternateStructureFileName<<std::endl;
         }
    }
    else if (getPdbNumberedStructureFileName() == getDefaultPdbNumberedStructureFileName()){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" No alternate PDB file specified. Pulling one from RCSB."<<std::endl; 
        std::string myUrlOfOriginFile = std::string("http://www.rcsb.org/pdb/files/") + getPdbId() + std::string(".pdb");
        pullFileWithCurl( myUrlOfOriginFile,  myDestinationFileNameWithPath); 
        if (access(myDestinationFileNameWithPath.c_str(), R_OK) == 0) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a structure file called "<<myDestinationFileNameWithPath <<std::endl;
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we  do NOT have read access to a  structure file called "<<myDestinationFileNameWithPath  <<" . This is a problem!"<< __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__  <<std::endl; exit(1);
        }
    } // else if (getPdbNumberedStructureFileName() == getAlignedStructureFileName()){ // getAlignedStructureFileName can only be called from a HomologJob
      //  std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Not sure we really expected this to occur. Maybe we want it to occur, but in any case getPdbNumberedStructureFileName() currently is not set up to return this."<<std::endl;
    //    exit(1);     
    //} 
    else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Unexplained error! "<<std::endl;
        exit(1);     
    }
};
*/


void  HomologJob::initializeSequencesFromPdb(std::string myPdbFileName){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Argument passed to function is >"<<myPdbFileName<<"<"<<std::endl;	 
    updBiopolymerClassContainer().loadSequencesFromPdb(myPdbFileName,false, "",false,true); 
    //updBiopolymerClassContainer().renumberPdbResidues(ResidueID("1"));
}
void  PrimaryJob::initializeSequencesFromPdb(std::string myPdbFileName){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Argument passed to function is >"<<myPdbFileName<<"<"<<std::endl;	 
    updBiopolymerClassContainer().loadSequencesFromPdb(myPdbFileName,false, "",false,true); 
    //updBiopolymerClassContainer().renumberPdbResidues(ResidueID("1"));
}


void PrimaryJob::setMutationVectorFromMutationString(){ //  loads the biopolymerClassContainer.mutationVector , using PrimaryJob's mutationString member, which should have been previously set.
    if (getMutationString().length() == 0){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The mutation string : "<< getMutationString() << " has length "<< getMutationString().length() <<" . We are treating this as an error. "<<std::endl; exit (1);
    }
    updBiopolymerClassContainer().setMutationVectorFromString(getMutationString()); 
    if (updBiopolymerClassContainer().updMutationVector().size() == 0){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" biopolymerClassContainer.mutationVector has size "<< updBiopolymerClassContainer().updMutationVector().size() <<" . This should not happen!"<<std::endl; exit(1);
    }
};

void  PrimaryJob::printBreederCommands(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" It's probably a bad idea to call this!"<<std::endl; exit(1);
    for (int i = 0; i < homologJobVector.size(); i++){
        homologJobVector[i].printBreederCommand(); 
    }
}

// List all chains in PrimaryJob
void PrimaryJob::listChains(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to list all chains in PrimaryJob :"<<std::endl; 
    for (auto it=chainAndComplexNumberMap.begin(); it!=chainAndComplexNumberMap.end(); ++it){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Chain : "<<it->first<<" belongs to complex "<< it->second <<std::endl;
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done listing all chains in PrimaryJob "<<std::endl; 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" doing the same from BiopolymerClassContainer : "<<std::endl; 
    updBiopolymerClassContainer().printBiopolymerInfo();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done with BiopolymerClassContainer  "<<std::endl; 
};

std::string insertBeforeComma(std::string & mainString, std::string insertedString){
    int commaPos = mainString.find(',');
    mainString.insert(commaPos,insertedString);
    return mainString;
}

std::string PrimaryJob::getComplexString(){
    std::string myComplexString = ","; // the complex string will in the end look like AB,CD where A and B are in complex 0, and C and D are in complex 1. So we start with the "," in the middle. Then we add chains from complex 0 on the left side, and chains from complex 1 on the right.  Order doesn't matter much here, except maybe aesthetically.
    for (auto it=chainAndComplexNumberMap.begin(); it!=chainAndComplexNumberMap.end(); ++it){
        if (it->second == 0){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to modify complex string : "<<myComplexString<<" . "<<std::endl;
            insertBeforeComma(myComplexString,it->first);    
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just added chain "<<it->first<< " of complex "<<it->second<<" on the left, albeit rightmost among complex 0. complex string is now : "<<myComplexString<<" . "<<std::endl;
        }
        else if (it->second == 1){
            myComplexString = myComplexString + it->first ;
        }
        else {std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that chain "<<it->first<<" belongs to complex "<<it->second <<" . Complex numbers are restricted to 0 and 1!"<<std::endl; exit(1);}
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<< __LINE__<<" Returning complex string : "<<myComplexString<<" . "<<std::endl;
    return myComplexString;
};

void PrimaryJob::loadChainAndComplexNumberMap(std::string complexString){
    int myComplexNumber = 0;
    for (int i = 0 ; i < complexString.length() ; i++){
        if (complexString.substr(i,1).compare(",") == 0){
            myComplexNumber++;
            if (myComplexNumber > 1){std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! Bad complex string! "<<std::endl; exit(1);}
        } else {
            // The character at position i is a chain ID. Add this along with its complex number (0 or 1) to chainAndComplexNumberMap:
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Adding chain "<<complexString.substr(i,1)<<" to complex number : "<<myComplexNumber<<std::endl;
            chainAndComplexNumberMap.insert(std::pair<std::string, int>(complexString.substr(i,1), myComplexNumber));
        }
        
    }
};
std::string PrimaryJob::getMutationStringFromBiopolymerClassContainer(){
   std::string breederFormattedMutationString =  updBiopolymerClassContainer().getFormattedMutationsString(std::string("-"));
   return breederFormattedMutationString;
}

std::string PrimaryJob::getMutationString(){return mutationString;};

void  PrimaryJob::printData(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" I am a PrimaryJob with following characteristics: "<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" PDB ID: "<<getPdbId()<<std::endl;
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" primaryToHomologChainIdMap: "<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" chainAndComplexNumberMap: "<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" chainAndComplexNumberMap.size(): "<<chainAndComplexNumberMap.size()<<std::endl;
    for (std::map <std::string, int>::iterator  it=chainAndComplexNumberMap.begin(); it!=chainAndComplexNumberMap.end(); ++it){
    //for (auto it=chainAndComplexNumberMap.begin(); it!=chainAndComplexNumberMap.end(); ++it){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Chain :"<< it->first <<" belongs to complex :  "<< it->second << " . " << std::endl;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
    } // of for
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
} // of printData()

void PrimaryJob::setWorkingDirectory(){
    //std::string systemCall1 =  std::string(" mkdir --verbose ") + overarchingDirectory + std::string(" ; chmod 777 ") + overarchingDirectory + " ; ";
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" About to issue :"<<std::endl<<systemCall1<<std::endl;
    //system(systemCall1.c_str());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
    myMkdir(overarchingDirectory);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
    /*
    if (!(opendir(overarchingDirectory.c_str()))){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" opendir failed to open directory "<<overarchingDirectory<<" . Will now create this directory.  " <<std::endl;
        const int dir_err = mkdir(overarchingDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (-1 == dir_err)
        {
            printf("Error creating directory!n");
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Failed to  create directory "<<overarchingDirectory<<"  " <<std::endl;
            exit(1);
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Successfully created directory "<<overarchingDirectory<<" " <<std::endl;
        }
    }
    if (access((overarchingDirectory ).c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a directory called "<< overarchingDirectory   <<" . So far so good."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! Found that we do NOT have read access to a directory called "<<  overarchingDirectory  <<"  "<<std::endl; exit(1);
    } */  
    std:string myWorkingDirectory = overarchingDirectory + std::string("/") + getPdbId() + std::string("/");
    dbConnection->setWorkingDirectory(myWorkingDirectory);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just set workingDirectory = >"<<dbConnection->getWorkingDirectory()<<"< "<<std::endl;
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just set workingDirectory = >"<<workingDirectory<<"< "<<std::endl;
    //std::string systemCall2 =  std::string(" mkdir --verbose ") + getWorkingDirectory() + std::string(" ; chmod 777 ") + getWorkingDirectory() + " ; ";
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" About to issue :"<<std::endl<<systemCall2<<std::endl;
    //(((system(systemCall2.c_str());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
    myMkdir(getWorkingDirectory());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" getWorkingDirectory() = >"<<getWorkingDirectory()<<"< "<<std::endl;   
    /*
    if (!(opendir(getWorkingDirectory().c_str()))){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" opendir failed to open directory "<<getWorkingDirectory()<<" . Will now create this directory.  " <<std::endl;
        const int dir_err = mkdir(getWorkingDirectory().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (-1 == dir_err)
        {
            printf("Error creating directory!n");
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Failed to  create directory "<<getWorkingDirectory()<<"  " <<std::endl;
            exit(1);
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<" Successfully created directory "<<getWorkingDirectory()<<" " <<std::endl;
        }
    }

    if (access(getWorkingDirectory().c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a directory called "<< getWorkingDirectory()   <<" . So far so good."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Heads up! Found that we do NOT have read access to a directory called "<< getWorkingDirectory()   <<"  "<<std::endl; exit(1);
    } 
    */  
}; // This method sets the working directory to the overarching directory +"/" + current object's pdbId .


bool homologJobsSharePrimaryChains ( HomologJob & homologJob1, HomologJob & homologJob2){
    bool match = 0;
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap1 = homologJob1.getPrimaryToHomologChainIdMap();   
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap2 = homologJob2.getPrimaryToHomologChainIdMap();   
    for (auto& it1: tempPrimaryToHomologChainIdMap1) {
        for (auto& it2: tempPrimaryToHomologChainIdMap2) {
            if ((it1. first).compare(it2. first) == 0){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob on the left ("<<homologJob1.getCachedPdbId() <<") has a primary chain ID "<< (it1. first) <<", whuile the HomologJob on the right ("<<homologJob2.getCachedPdbId() <<") has a matching chain ID : "<< (it2. first)<<" . Returning True."<<std::endl;
                return 1;
            } // of if
        } // of for auto it2
    } // of for auto it1
    return 0;
};
bool homologJobsShareHomologChains ( HomologJob & homologJob1, HomologJob & homologJob2){
    bool match = 0;
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap1 = homologJob1.getPrimaryToHomologChainIdMap();   
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap2 = homologJob2.getPrimaryToHomologChainIdMap();   
    for (auto& it1: tempPrimaryToHomologChainIdMap1) {
        for (auto& it2: tempPrimaryToHomologChainIdMap2) {
            if ((it1.second).compare(it2.second) == 0){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob on the left ("<<homologJob1.getPdbId() <<") has a homolog chain ID "<< (it1. second) <<", while the HomologJob on the right ("<<homologJob2.getPdbId() <<") has a matching chain ID : "<< (it2.second)<<" . Returning True."<<std::endl;
                return 1;
            } // of if
        } // of for auto it2
    } // of for auto it1
    return 0;
};

bool homologJobsAreIdentical ( HomologJob & homologJob1, HomologJob & homologJob2){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    bool identical = 1; // Assume they are identical. If any difference is found, conclude the oppposite.
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap1 = homologJob1.getPrimaryToHomologChainIdMap();   
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap2 = homologJob2.getPrimaryToHomologChainIdMap();   
    for (auto& it1: tempPrimaryToHomologChainIdMap1) {
        if (it1.second != tempPrimaryToHomologChainIdMap2[it1.first]) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The two homolog jobs tested are NOT identical. "<<std::endl;
            identical = 0; return identical;} 
    }
    for (auto& it2: tempPrimaryToHomologChainIdMap2) {
        if (it2.second != tempPrimaryToHomologChainIdMap1[it2.first]) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The two homolog jobs tested are NOT identical. "<<std::endl;
            identical = 0; return identical;} 
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The two homolog jobs tested are identical. "<<std::endl;
    return identical;
};



// This function loops through the primaryToHomologChainIdMap of the homologJob. Returns 1 if any of the child or parent chains are repeated.
bool homologJobHasRepeatedParentOrChildChainsOrIncompatibleChainCount( HomologJob & homologJob ){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Homology job PDB ID :" <<homologJob.getPdbId()<<std::endl;
    ////// Insert the above
    bool identical = 1; // Assume they are identical. If any difference is found, conclude the oppposite.
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap1 = homologJob.getPrimaryToHomologChainIdMap();   
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    //std::map <std::string , std::string> tempPrimaryToHomologChainIdMap2 = primaryToHomologChainIdMap;   
    std::map <std::string , std::string>::iterator beginning = tempPrimaryToHomologChainIdMap1.begin();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    std::map <std::string , std::string>::iterator ending    = tempPrimaryToHomologChainIdMap1.end  ();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Length of tempPrimaryToHomologChainIdMap1.size() = "<<tempPrimaryToHomologChainIdMap1.size() <<std::endl;
    for (std::map <std::string , std::string>::iterator it1 = beginning; it1 != ending; it1++){ //auto& it1: tempPrimaryToHomologChainIdMap1) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1 has  parent chain : "<<it1->first <<std::endl;        
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1 has  child  chain : "<<it1->second <<std::endl;        

        if (it1 != ending) {
            std::map <std::string , std::string>::iterator it2 = it1; it2++;
            if (it2 == ending) {
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it2 == tempPrimaryToHomologChainIdMap1.end()  "<<std::endl; //exit(1);
            }
            //std::map <std::string , std::string>::iterator it2 = it1; it2++;
            for (; it2 != ending; it2++) {
                //++it2;
                if (it2 == ending) {
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it2 == tempPrimaryToHomologChainIdMap1.end  () ! This should not happen......"<<std::endl; exit(1);
                }
            //for (; it2 != ending; it2++	){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Comparing parent chain it1->first "<<it1->first << " vs. "<<std::endl ;
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it2->first : "<<it2->first<<std::endl;        
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Comparing child chains : "<<it1->second << " and "<<std::endl ;
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it2->second : "<<it2->second<<std::endl;        
                if (it1->first == it2->first){
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The provided homologJob has parent chain "<<it1->first<<" ocurring at least twice."<<std::endl;
                    return 1;     
                }
                if (it1->second == it2->second){
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The provided homologJob has child chain "<<it1->second<<" ocurring at least twice."<<std::endl;
                    return 1;     
                }
            } //while (it2 != ending);
        }
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The provided homologJob has no repeated parent or child chains "<<std::endl;
    return 0;
}

// This function loops through the primaryToHomologChainIdMap of the homologJob. Returns 1 if any of the child or parent chains are repeated.
bool PrimaryJob::homologJobHasChainsWithUnsatisfactorySequenceIdentity( HomologJob & homologJob ,  bool sequenceIdentityIsNull,  double mySequenceIdentityCutoff = SEQUENCEIDENTITYCUTOFF  ){
    std::map <std::string , std::string> tempPrimaryToHomologChainIdMap1 = homologJob.getPrimaryToHomologChainIdMap();   
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    std::map <std::string , std::string>::iterator beginning = tempPrimaryToHomologChainIdMap1.begin();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    std::map <std::string , std::string>::iterator ending    = tempPrimaryToHomologChainIdMap1.end  ();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Length of tempPrimaryToHomologChainIdMap1.size() = "<<tempPrimaryToHomologChainIdMap1.size() <<std::endl;
    for (std::map <std::string , std::string>::iterator it1 = beginning; it1 != ending; it1++){ 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1 has  parent chain : "<<it1->first <<std::endl;        
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1 has  child  chain : "<<it1->second <<std::endl;        
        if (dbConnection->countEntriesInMatchingChains(getPdbId(),homologJob.getPdbId(), it1->first, it1->second )){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
            if (dbConnection->chainSequenceAlignmentIsSatisfactory(getPdbId(),homologJob.getPdbId(), it1->first, it1->second, mySequenceIdentityCutoff)){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<it1->second <<std::endl;
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that parent PDB ID "<< getPdbId() << " chain "<< it1->first <<" vs. child PDB ID "<<homologJob.getPdbId() << " chain "<< it1->second << " have a satisfactory sequence identity. Moving on to the next one. "<<std::endl;
            } else {
                if (dbConnection->matchingChainsValueIsNull(getPdbId(),homologJob.getPdbId(), it1->first, it1->second, "sequenceIdentity"  )){
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that parent PDB ID "<< getPdbId() << " chain "<< it1->first <<" vs. child PDB ID "<<homologJob.getPdbId() << " chain "<< it1->second << " have a sequence identity of NULL. Moving on to the next one. "<<std::endl;
                  
                } else {
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that parent PDB ID "<< getPdbId() << " chain "<< it1->first <<" vs. child PDB ID "<<homologJob.getPdbId() << " chain "<< it1->second << " have an unsatisfactory sequence identity. Returning TRUE.  "<<std::endl;
                    return 1;
                }
            }
        }
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found no unsatisfactory sequence identities for parent PDB ID "<< getPdbId() << " vs. child PDB ID "<<homologJob.getPdbId() << std::endl; 
    return 0;
}

void PrimaryJob::loadHomologJobVectorFromFasta( std::string myChain){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myChain = "<<myChain<<std::endl;//shares a primary chain ID with an existing HomologJob. In the future we may simply ignore this, but for now we are going to consider this an error.  "<<std::endl;
    std::string sequence = getSequence(myChain); 
    std::string fastaExecutable =  breederParameterReader.fastaExecutable;
    std::string fastaTempPreDirectory = breederParameterReader.fastaTempDirectory + "/" + getPdbId() + "/"  ;
    std::string fastaTempDirectory =  breederParameterReader.fastaTempDirectory + "/" + getPdbId() + "/" + myChain + "/"  ; 
    double expupperlim = 0.00000000001;
    std::string fastaShortResultsFile = fastaTempDirectory + std::string("/fasta-pg.ids.txt");
    if (access(fastaShortResultsFile.c_str(), R_OK) == 0) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that we have read access to a file called "<<fastaShortResultsFile<<" . So we will not rerun fasta. If you think this file is outdated, please delete it to force a rerun. "<<std::endl;
    } else {
        std::ostringstream expupperlimStringStream;
        expupperlimStringStream << std::setprecision(14) << expupperlim;
        // I had a problem where on pe1 sort is /usr/bin/sort whereas elsewhere it might be /bin/sort. So I am just calling "sort" which is dangerous if the non-login shell can't find it.

        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        myMkdir(fastaTempPreDirectory);
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        myMkdir(fastaTempDirectory);
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        std::string fastaCommand = " cd " + fastaTempDirectory + " ;  rm " + fastaTempDirectory  +"/fasta-*  ; " + fastaExecutable + " --email samuelfloresc@gmail.com --program ssearch --database pdb --stype protein --expupperlim " +(expupperlimStringStream.str() )  + " --sequence  " + sequence + " ;  sort fasta-*.ids.txt  |  /usr/bin/uniq > " + fastaShortResultsFile;
        //std::string fastaCommand = "mkdir --verbose " + fastaTempPreDirectory + " ; mkdir --verbose " + fastaTempDirectory + " ; cd " + fastaTempDirectory + " ;  rm " + fastaTempDirectory  +"/fasta-*  ; " + fastaExecutable + " --email samuelfloresc@gmail.com --program ssearch --database pdb --stype protein --expupperlim " +(expupperlimStringStream.str() )  + " --sequence  " + sequence + " ;  sort fasta-*.ids.txt  |  /usr/bin/uniq > " + fastaShortResultsFile;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Issuing:"<<std::endl<<fastaCommand<<std::endl;
        system(fastaCommand.c_str()) ;
    }
    ifstream inputFile(fastaShortResultsFile);

    // test file open

    if (inputFile){
	    std::string  value = "";
            while (std::getline(inputFile,value))
	    //while ( inputFile >> value )
	    {
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Processing value from fasta run: >"<<value<<"< "<<std::endl;
	       HomologJob *myHomologJob = new HomologJob();
	       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(*myHomologJob) = "<< sizeof(*myHomologJob) <<std::endl; 
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
	       myHomologJob->setParentPrimaryJobPointer(this);
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to test getParentPrimaryJobPointer by calling its printData :"<<std::endl;
               (*myHomologJob->getParentPrimaryJobPointer()).printData();
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Compare that to this printData :"<<std::endl;
               printData();
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
               //SCF debug this:
	       myHomologJob->setCachedPdbId(value.substr(4,4));
               // Turns out we get too many mysql connections if we initializeDbConnection here.
               
               //myHomologJob->initializeDbConnection((*(myHomologJob->getParentPrimaryJobPointer())).breederParameterReader, (*(myHomologJob->getParentPrimaryJobPointer())).breederParameterReader. jobId    ); // Immediately follow up with changing jobId!
               // myHomologJob->setPdbId(value.substr(4,4));

               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<myHomologJob->getCachedPdbId()<<endl;
               //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<myHomologJob->dbConnection->getCachedPdbId()<<endl;
               //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<myHomologJob->dbConnection->getJobID()<<endl;

               if (myHomologJob->getCachedPdbId() != value.substr(4,4)) {
                   std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! The jobID :"<<myHomologJob->getCachedPdbId() << " does not match the input PDB ID : "<<  value.substr(4,4) << std::endl; exit(1);
               }
               else     std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Confirmed that the jobID :"<<myHomologJob->getCachedPdbId()<< "  matches the input PDB ID : "<<  value.substr(4,4) << std::endl; 
               
               if (myHomologJob->getCachedPdbId() != value.substr(4,4)) {
                   std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! The jobID : myHomologJob->getCachedPdbId() : "<<myHomologJob->getCachedPdbId() << " does not match the input PDB ID : "<<  value.substr(4,4) << std::endl; exit(1);
               }
               else     std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Confirmed that the jobID : myHomologJob->getCachedPdbId() :"<<myHomologJob->getCachedPdbId()<< "  matches the input PDB ID : "<<  value.substr(4,4) << std::endl; 

               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
 	       myHomologJob->setCorrespondingChain( myChain , value.substr(9,1) );
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
               if (myHomologJob->getCachedPdbId().compare("PDB:") != 0){ // Sometimes there is a "PRE_PDB:" entry in the list which should be discarded
                   if (myHomologJob->getCachedPdbId() == myHomologJob->getParentPrimaryJobPointer()->getPdbId()){
                       if (!(myHomologJob->getParentPrimaryJobPointer()->getHasBeenCloned())){
                           std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                           myHomologJob->copyChainsFromParent();
                           //myHomologJob->printData();
                           std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
	                   addHomologJob(*myHomologJob); // This procedure automatically checks that this job does not already exist. If one or more with the same PDB already exists, consider merging, and with which one. Alternatively, consider creating a new one.  There may be circumstances under which a validation step would fail, think about  this.
	                   std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(*myHomologJob) = "<< sizeof(*myHomologJob) <<std::endl; 
		  
                       } else {
                           std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The PrimaryJob has already been cloned and presumably submitted. Discarded the current HomologJob"<<std::endl;
                       } // if the PrimaryJob has already been cloned, don't bother adding any additional duplicates. In the future we should look at the case of additional complexes in the same PDB file. Possibly self-children could be treated as indistinct from children which have different PDB IDs from their parent. 
                   } else {
    
   
                       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Adding job."<<std::endl;
                       //myHomologJob->printData(); // This can't be done here because the DB connection has not been initialized.
	               addHomologJob(*myHomologJob); // This procedure automatically checks that this job does not already exist. If one or more with the same PDB already exists, consider merging, and with which one. Alternatively, consider creating a new one.  There may be circumstances under which a validation step would fail, think about  this.
                       // Now record the maximum EValue for these two chains. We are not parsing the actual EValue for now, that is unknown to us at this point.  
                       dbConnection->setMatchingChainsValue(getPdbId(), myHomologJob->getCachedPdbId(), myChain, myHomologJob->getCorrespondingChain(myChain) , std::string ("maxEValue") ,expupperlim);
	               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(*myHomologJob) = "<< sizeof(*myHomologJob) <<std::endl; 
                   }
               } else {std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Detected that this entry: "<<value<<" in "<<fastaShortResultsFile<<"  has a fragment like : "<<myHomologJob->getCachedPdbId()<<" . Skipping this entry."<<std::endl; }
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
               //myHomologJob->dbConnection->close(); // We can't have too many open connections. setJobID has been called, that is enough for now. 
               myHomologJob->printPrimaryToHomologChainIdMap(); 
    /**/
	    } // of while
        } // of if inputFile                  
    inputFile.close();
    } // of  PrimaryJob::loadHomologJobVectorFromFasta

void PrimaryJob::addHomologJob(HomologJob & myHomologJob){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  "<<std::endl; 
    if (homologJobHasRepeatedParentOrChildChainsOrIncompatibleChainCount(myHomologJob)){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homologJob provided has child-parent incompatible chain count, or repeated child or parent chains. Discarding this homologJob. Actually we should never get to this point."<<std::endl;
        return;
    };
    bool mySequenceIdentityIsNull = 0;
    if (homologJobHasChainsWithUnsatisfactorySequenceIdentity(myHomologJob, mySequenceIdentityIsNull, SEQUENCEIDENTITYCUTOFF ) ){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homologJob provided has at least one chain which has unsatisfactory sequence identity, less than  "<<SEQUENCEIDENTITYCUTOFF<<",  with respect to its matched parent chain. Or maybe it simply hasn't been calculated and is null .. checking this.."<<std::endl;
        if (mySequenceIdentityIsNull) {
             std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Actually the sequence identity is just NULL. So we can continue with this job for now."<<std::endl;
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The sequence identity was actually NOT NULL. So we will discard this. "<<std::endl;
            return;
        }   
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homologJob provided either has fully satisfactory  sequence identity , satisfying "<<SEQUENCEIDENTITYCUTOFF<<", for all child chains with respect to its matched parent chains, or alternatively sequence identity has not yet been computed. Continuing with this job. "<<std::endl;
    }
    vector<int> matchingHomologJobIndices;matchingHomologJobIndices.clear();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"homologJobVector.size() "<<homologJobVector.size()<< std::endl;
    for (int i = 0;i < homologJobVector.size() ; i++){
        // If the PDB ID of the new job matches one in the homologJobVector, then this is a suspected duplicate. Add to matchingHomologJobIndices for further investigation:
        if (homologJobVector[i].getCachedPdbId().compare(myHomologJob.getCachedPdbId()) == 0) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" We found an existing HomologJob with PDB ID "<<homologJobVector[i].getCachedPdbId()<<" which matches a new HomologJob with PDB ID "<<myHomologJob.getCachedPdbId()<<" . We will determine whether this is a duplicate by examining the chains in more detail."<<std::endl;
            matchingHomologJobIndices.push_back(i); 
        }
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" matchingHomologJobIndices.size() = "<<matchingHomologJobIndices.size()<<std::endl;
    if (matchingHomologJobIndices.size() == 0){ // No such HomologJob exists in homologJobVector, so add it.
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"homologJobVector.size() "<<homologJobVector.size()<< std::endl;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"homologJobVector.max_size() "<<homologJobVector.max_size()<< std::endl;
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" homologJobVector.capacity() = "<<homologJobVector.capacity()<<std::endl; 
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(myHomologJob) = "<< sizeof(myHomologJob) <<std::endl; 
        if (!(homologJobVector.size() % 100)) {homologJobVector.reserve(homologJobVector.size()+102);}
        //if (homologJobVector.capacity()-homologJobVector.size()) < 100) {homologJobVector.reserve(homologJobVector.size()+100);}
        //homologJobVector.reserve(homologJobVector.size()+2); // Increase the capacity so we can store a couple ofmore elements in addition to those already in the vector
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" homologJobVector.capacity() = "<<homologJobVector.capacity()<<std::endl; 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Pushing back  homologJobVector "<<std::endl; 
        homologJobVector.push_back(myHomologJob); 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  "<<std::endl; 
    } else {
        for (int i = 0;i < matchingHomologJobIndices.size() ; i++){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Matching index: "<<matchingHomologJobIndices[i]<<std::endl;
            if (homologJobsAreIdentical(homologJobVector[matchingHomologJobIndices[i]], myHomologJob)){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homolog job in question has all the same chains as an existing job. Will not add this job."<<std::endl;
                return; // There is already a job like this. Nothing more to do.
            } else {
                // Do nothing. Do not add the job yet. Let the loop continue looking for an exact duplicate.
            } // of if
            
        } // of for
       //(myHomologJob); // The job is not an exact duplicate. Add the job.
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" We have checked The homolog job in question against "<< matchingHomologJobIndices.size() <<" existing homologyJob's with similar characteristics. We found NO exact duplicates. Moving on to the next test."<<std::endl;
       //(homologJobVector[matchingHomologJobIndices[i]]).mergePrimaryToHomologChainIdMaps(myHomologJob.getPrimaryToHomologChainIdMap());
       //nearestMatchingIndex = i;
       //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Successfully merged myHomologJob with  homologJobVector["<<i<<"] ."<<std::endl;
       for (int i = 0;i < matchingHomologJobIndices.size() ; i++){
           std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Matching index: "<<matchingHomologJobIndices[i]<<std::endl;
           if (!(homologJobsSharePrimaryChains(homologJobVector[matchingHomologJobIndices[i]], myHomologJob))){
               ////// First, clone the homologJob to be merged, so it does not disappear from view, unable to be paired with anyone else::
               homologJobVector.push_back(homologJobVector[matchingHomologJobIndices[i]]); 
               // note the cloning of myHomologJob is done after this if/else
               ////// This ensures the individual, partial HomologJobs will be around for other chains that may want to merge with them.
               // Note all this cloning leaves a bunch of incomplete HomologJobs at the end. That is OK, complexMatchesParent() is called during printCorrespondenceTable() which means these will all be culled.
               // Next, perform the merge. The two constitutent HomologJobs would have ceased to exist, had they not been cloned above.  
               (homologJobVector[matchingHomologJobIndices[i]]).mergePrimaryToHomologChainIdMaps(myHomologJob.getPrimaryToHomologChainIdMap());
               //nearestMatchingIndex = i;
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done with attempted merge of myHomologJob with  homologJobVector["<<i<<"] ."<<std::endl;
               //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Successfully merged myHomologJob with  homologJobVector["<<i<<"] ."<<std::endl;
               //break;
           } else {
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" homologJobsSharePrimaryChains returned true, Not merging."<<std::endl;
               // The job needs to continue existing so it can find future potential pairings:
               //(homologJobVector[matchingHomologJobIndices[i]]); 
           }// of if
           //////
           // In the first case (if) the newcomer job needs to be cloned. In the second case, the newcomer job needs to be kept around in case it can merge with someone else. In either case, the procedure is:
           
           //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" issuing addHomologJob( myHomologJob)."<<std::endl;
           //;
           //////
       } // of for
       for (int i = 0;i < matchingHomologJobIndices.size() ; i++){
           if (homologJobsAreIdentical(homologJobVector[matchingHomologJobIndices[i]], myHomologJob)){
               std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that myHomologJob already exists in homologJobVector. Cutting out of addHomologJob"<<std::endl;
               return;
           }
       } 
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that myHomologJob does NOT exist in homologJobVector. Adding it now."<<std::endl;
       homologJobVector.push_back(myHomologJob); 
       //addHomologJob(myHomologJob); // This will not be an infinite loop because there is a homologJobsAreIdentical call above.
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    }
};

void mySystemCall(std::string commandString){
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to make system call for string : >"<<commandString<<"< "<<std::endl;
    int systemCallReturnValue ;
    systemCallReturnValue = system(commandString.c_str());
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just completed system call for string : >"<<commandString<<"< "<<std::endl;
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Got return code of : "<<systemCallReturnValue<<std::endl;
}

std::string HomologJob::getJobName(){
    std::string myJobName = getPdbId() + "." + getComplexString() + "." +getParentPrimaryJobPointer()->getPdbId() +"." + getParentPrimaryJobPointer()->getComplexString() + getParentPrimaryJobPointer()->breederParameterReader.oneMutationString;
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Generated job name : >"<<myJobName<<"< "<<std::endl;
    return myJobName;
}

void HomologJob::submitCommandtoSlurm( std::string commandString , std::string myJobName ="" ){
    ofstream mySlurmJobFile;
    if (myJobName == "") {
        myJobName = getJobName();
        std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just set -J jobname parameter to "<<myJobName<<std::endl;
    }
    std::string mySlurmJobFileName = getParentPrimaryJobPointer()->breederParameterReader.workingDirectory + "/" + myJobName + ".job";
    //std::string mySlurmJobFileName = getParentPrimaryJobPointer()->breederParameterReader.workingDirectory + "/" + getPdbId() + "." + getComplexString() + "." + getParentPrimaryJobPointer()->breederParameterReader.oneMutationString + ".job";
    //std::string mySlurmLogFileName = getParentPrimaryJobPointer()->breederParameterReader.workingDirectory + "/" getPdbId() + "." + getComplexString() + "." + getParentPrimaryJobPointer()->breederParameterReader.oneMutationString + ".log";
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to create slurm job in file "<<mySlurmJobFileName<<std::endl;
    mySlurmJobFile.open(mySlurmJobFileName);
    mySlurmJobFile <<"#!/bin/bash -l " ;
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH -J "<< myJobName ;//myJobName;
    //getPdbId()<<"."<<getComplexString()<<"."<< getParentPrimaryJobPointer()->getPdbId()<< "."<<getParentPrimaryJobPointer()->getComplexString()<<"." <<getParentPrimaryJobPointer()->getMutationString();
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH -A X ";
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH -t 20:00:00 " ; // 2 then 4 then 10 hours was not enough for some 3NSS jobs
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH --mem 4000 " ;
    mySlurmJobFile << std::endl;
    //mySlurmJobFile <<"#SBATCH --ntasks=1
    //mySlurmJobFile <<"#SBATCH --ntasks-per-node=1
    mySlurmJobFile <<"#SBATCH --cpus-per-task=1 " ;
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH -N 1                  " ;
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH -n 1                 " ;
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH --oversubscribe                 " ;
    mySlurmJobFile << std::endl;
    mySlurmJobFile <<"#SBATCH -o "<< mySlurmJobFileName << ".%j.slurm.out"; 
    mySlurmJobFile << std::endl;
    mySlurmJobFile << std::endl;
    mySlurmJobFile << commandString<<std::endl;
    mySlurmJobFile.close();
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    mySystemCall("chmod +x "+mySlurmJobFileName+"; "+mySlurmJobFileName);
    //mySystemCall(std::string("sbatch ")+mySlurmJobFileName);
    std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 

}

void PrimaryJob::spawnSingleHomologyScannerRunsFromHomologJobVector(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    int numMatchingComplexes = 0;
    bool firstTime = 1;
    while(homologJobVector.size()){
        if (homologJobVector.back().getCachedPdbId().size() != 4){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The pdb ID is the wrong size, "<<homologJobVector.back().getCachedPdbId().size(); //<<std:endl;
            std::cout<<std::endl;
            exit(1); 
        }else{
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The pdb ID is the right size, "<<homologJobVector.back().getCachedPdbId().size();
            std::cout<<std::endl;
        };
        if (! (homologJobVector.back().complexMatchesParent())) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homolog job does NOT have chains matching those in the parent. Not moving on with this job."<<std::endl;
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homolog job has chains matching those in the parent. Moving on with this job."<<std::endl;
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            int systemCallReturnValue = -11111;
            std::string homologyScannerSingleRunCommand = "/usr/local//bin/homologyScanner " ; // There should be a config file for variables like, this, and the sequence identity cutoff.
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just set homologyScannerSingleRunCommand = >"<<homologyScannerSingleRunCommand<<"< "<<std::endl;
            homologyScannerSingleRunCommand += commonBreederParameters();
            homologyScannerSingleRunCommand += " -PDBID ";
            homologyScannerSingleRunCommand += getPdbId();
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologyScannerSingleRunCommand += " -SINGLEHOMOLOGYSCANNERRUN -CHAINSINHOMOLOGCOMPLEX ";
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologyScannerSingleRunCommand += homologJobVector.back().getComplexString();// definitely not breederParameterReader.chainsInHomologComplex. That one would be right in the leaf job, we are in the overarching job.
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologyScannerSingleRunCommand += " -HOMOLOGPDBID ";
            homologyScannerSingleRunCommand += homologJobVector.back().getPdbId();
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologyScannerSingleRunCommand += " -ONEMUTANT " + breederParameterReader.oneMutationString; // getMutationString(); 
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            //homologJobVector.back().setWorkingDirectory();
            homologyScannerSingleRunCommand += " -WORKINGDIRECTORY " + breederParameterReader.workingDirectory;//  getWorkingDirectory() + std::string("/") +  homologJobVector.back().getPdbId()  ;        
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologyScannerSingleRunCommand += " -CHAINSINCOMPLEX " + getComplexString() ; // from Primary
            homologyScannerSingleRunCommand += " &> " + breederParameterReader.workingDirectory + "/" + homologJobVector.back().getJobName() + ".leaf-job.log"; //  breederParameterReader.oneMutationString + "." + homologJobVector.back().getPdbId() + "." + homologJobVector.back().getComplexString() + ".leaf-job.log";
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            // mySystemCall(homologyScannerSingleRunCommand);
            homologJobVector.back().submitCommandtoSlurm(homologyScannerSingleRunCommand);
            std::cout << __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        }
        homologJobVector.pop_back();
    }
}
        //homologyScannerSingleRunCommand += -FASTAEXECUTABLE /usr/local//fasta_lwp/fasta_lwp.pl -FASTATEMPDIRECTORY /usr/local//fasta_lwp///temp/ -BREEDEREXECUTABLE /usr/local/MMB/bin/breeder -BREEDERMAINDIRECTORY /home/sam/svn/breeder -DATABASE mmb -MMBEXECUTABLE /usr/local/MMB/bin/MMB -LASTSTAGE 1 -FOLDXSCRIPT /usr/local/MMB/bin/run-foldx.3.pl -FOLDXEXECUTABLE //usr/local//foldx/foldx -SQLSERVER localhost -SQLEXECUTABLE /usr/bin/mysql -SQLPASSWORD mMBc9IU5@r -USER root -SQLUSER mmbcgi -JOBLIBRARYPATH /usr/lib/x86_64-linux-gnu/blas:/home/sam/svn/breeder/build:/usr/local/MMB/lib -REPORTINGINTERVAL 0.000001 -NUMREPORTINGINTERVALS 2 -FLEXIBILITYWINDOWOFFSET 2 -TEMPERATURE 298 -ID homoScan.1 -ONEMUTANT A-107-A -WORKINGDIRECTORY /data//runs/homoScan.1/3NSS  -CHAINSINCOMPLEX A,B   -PDBID 3NSS  -SQLSYSTEM MySQL -ACCOUNT X -MOBILIZERRADIUS 0.0 -PARTITION core ";

int PrimaryJob::createSingleHomologJobAndAddToVector( std::string myComplexChains, std::string myHomologPdbId){

    // test file open

    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Processing value from fasta run: >"<<value<<"< "<<std::endl;
    HomologJob myHomologJob; // = new HomologJob();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(myHomologJob) = "<< sizeof(myHomologJob) <<std::endl; 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    //SCF debug this:
    myHomologJob.setCachedPdbId(breederParameterReader.homologPdbId);
    myHomologJob.setParentPrimaryJobPointer(this);

    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<myHomologJob.getCachedPdbId()<<endl;
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<myHomologJob.dbConnection->getCachedPdbId()<<endl;
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<myHomologJob.dbConnection->getJobID()<<endl;

    if (myHomologJob.getCachedPdbId().size() != 4) {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! The PDBID :"<<myHomologJob.getCachedPdbId() << " has size : "<<  myHomologJob.getCachedPdbId().size() << std::endl; exit(1);
    }
    else     std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Confirmed that the PDB :"<<myHomologJob.getCachedPdbId()<< "  has the correct size of  : "<< 4  << std::endl; 
    
    // set complex chains around here..
    if (breederParameterReader.chainsInHomologComplex.length() == breederParameterReader.chainsInMutatedSubunit.length()){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Confirmed that Primary and Homolog jobs have the same number of chains."<<std::endl;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Confirmed that PrimaryJob has "<<breederParameterReader.chainsInHomologComplex<<" , while HomologJob has "<<breederParameterReader.chainsInMutatedSubunit<<" . These are incompatible lengths. Aborting this HomologJob "<<std::endl; return(1); // Abort this job. Exit this function without dying, so that PrimaryJob.dbConnection can close.
    }
    std::string mySeparator = ",";
    for (int i = 0; i < breederParameterReader.chainsInHomologComplex.length(); i++){
        if ((breederParameterReader.chainsInMutatedSubunit.substr(i,1) == mySeparator) !=  (breederParameterReader.chainsInHomologComplex.substr(i,1)  == mySeparator)){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Homolog job has "<<breederParameterReader.chainsInHomologComplex<<" , while primary Job has "<<breederParameterReader.chainsInMutatedSubunit<<" . The separator, \""<<mySeparator<<"\" is not in compatible positions in these two strings "<<std::endl;
        } else if ((breederParameterReader.chainsInMutatedSubunit.substr(i,1) == mySeparator) && (breederParameterReader.chainsInHomologComplex.substr(i,1)  == mySeparator)){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" PrimaryJob has "<<breederParameterReader.chainsInHomologComplex<<" , while HomologJob has "<<breederParameterReader.chainsInMutatedSubunit<<" . Doing nothing, as current charater is the separater, \""<<mySeparator<<"\" . "<<std::endl;
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Homolog Job has "<<breederParameterReader.chainsInHomologComplex<<" , while primary has "<<breederParameterReader.chainsInMutatedSubunit<<" . Right now we are linking Primary chain "<< breederParameterReader.chainsInMutatedSubunit.substr(i,1) <<" to Homolog chain "<<breederParameterReader.chainsInHomologComplex.substr(i,1)<<" . "<<std::endl;
            myHomologJob.setCorrespondingChain( breederParameterReader.chainsInMutatedSubunit.substr(i,1), breederParameterReader.chainsInHomologComplex.substr(i,1) );
        }
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    myHomologJob.printPrimaryToHomologChainIdMap(); 
    addHomologJob(myHomologJob); // This procedure automatically checks that this job does not already exist. If one or more with the same PDB already exists, consider merging, and with which one. Alternatively, consider creating a new one.  There may be circumstances under which a validation step would fail, think about  this.
    return 0;
} 
/**/

void PrimaryJob::printCorrespondenceTable(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::string mysqlAggregationCommand = "select SUM(foldx_energy - foldx_energy_wild_type)/count(*) from results where jobName = \"" + dbConnection->getJobID() + "\" AND ("; //AVE function returns a mysql error for some reason.
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    //
    int numMatchingComplexes = 0;
    //std::unique_ptr<HomologJob> tempHomologJobUniquePtr(tempHomologJob);
    // HomologJob *tempHomologJob = new HomologJob();
    //bool firstTime = 1;
    while(homologJobVector.size()){
        //if (!(firstTime)){homologJobVector.pop_back();}
        //firstTime = 0; 
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The homologJobVector currently has size: "<<homologJobVector.size()<<std::endl;
            struct sysinfo memInfo;
            long long totalVirtualMem;
            long long physMemUsed ;
            sysinfo (&memInfo);
            totalVirtualMem = memInfo.totalram;
            //Add other values in next statement to avoid int overflow on right hand side...
            totalVirtualMem += memInfo.totalswap;
            totalVirtualMem *= memInfo.mem_unit;
            physMemUsed = memInfo.totalram - memInfo.freeram;
            //Multiply in next statement to avoid int overflow on right hand side...
            physMemUsed *= memInfo.mem_unit;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Physical memory used : "<<physMemUsed<<" bytes "<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Physical memory used : "<<1.0*physMemUsed/(1024*1024*1024)<< " GB "<<std::endl;


        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        //homologJobVector.back().printData();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        //homologJobVector.back().printPrimaryToHomologChainIdMap();
    
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Checking the HomologJob in question:"<<std::endl;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   

        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        if (homologJobVector.back().complexMatchesParent()){ // this check is previously done in spawnSingleHomologyScannerRunsFromHomologJobVector . But I guess no harm in testing again.
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob "<<homologJobVector.back().getCachedPdbId()<<" has chains corresponding to those in the parent. We will continue with this job."<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Main job PDB ID: "<<getPdbId()<<" , mutationString: "<<getMutationString()<<std::endl ;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<homologJobVector.back().getCachedPdbId()<<" , mutationString: "<<homologJobVector.back().getMutationString()<< " HomologJob complex string: "<< homologJobVector.back().getComplexString()<< std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
            homologJobVector.back().initializeDbConnection((*(homologJobVector.back().getParentPrimaryJobPointer())).breederParameterReader,  ((*(homologJobVector.back().getParentPrimaryJobPointer())) ).breederParameterReader.jobId   );



            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< std::endl; 
            homologJobVector.back().setWorkingDirectory(); // This sets the workingDirectory to parent's working directory + "/" + tempHomologJob's pdbId //was just above call to homologJobVector.back().initializeDbConnection. But requries mysql now.
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< std::endl; 
            homologJobVector.back().setPdbId(homologJobVector.back().getCachedPdbId()); // There is a danger that if this step silently fails, we are accessing the parent's data
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< std::endl; 




            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " HomologJob cached PDB ID: "<<homologJobVector.back().getCachedPdbId()<<endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " HomologJob mysql  PDB ID: "<<homologJobVector.back().dbConnection->getPdbId()<<endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " HomologJob jobName      : "<<homologJobVector.back().dbConnection->getJobID()<<endl;
            homologJobVector.back().fetchPdb(); // This sets structureFileName (with full path : workingDirectory),  creates a directory named workingDirectory, then fetches pdbId + ".pdb" and puts it in workingDirectory
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<std::endl;

            //SCF got rid of this 19 April homologJobVector.back().dbConnection->setPdbStatusFail(); // Preemptively set status to FAIL in case the next step fails. 

            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
            // tried this in writeRenumberedPdbIfAbsent() 
            //myChromosome.populateSequenceTable(*dbConnection);
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
	    // This was causing a seg fault, had to pointer-ize ParameterReader instance
            homologJobVector.back().writeRenumberedPdbIfAbsent();
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
	    //SCF mark was here
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
            HomologJob tempHomologJob = homologJobVector.back();
            tempHomologJob.initializeSequencesFromPdb(homologJobVector.back().getPdbNumberedStructureFileName());
            tempHomologJob.translateMutationVectorFromParent();
            Chromosome tempChromosome (tempHomologJob.getMutationString(),tempHomologJob.updBiopolymerClassContainer() );
            tempChromosome.populateSequenceTable(*(tempHomologJob.dbConnection)); // does this need an asterisk?
            // Here is the problem with geting the pdb numberd residues. homolog jobs from the getgo are initialized with renumberd residue numbers:
            homologJobVector.back().initializeSequencesFromPdb(homologJobVector.back().getRenumberedPdbFileName());//      updBiopolymerClassContainer().loadSequencesFromPdb(myPdbFileName,false, "");
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
            //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // Not sure if we should wait even longer to declare success. here all we are saying is that the coordinate matching worked, which is the usual problem, and that sequence extractiohn subsequently worked


            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
            //homologJobVector.back().initializeSequencesFromPdb(homologJobVector.back().chooseStructureFileName());//      updBiopolymerClassContainer().loadSequencesFromPdb(myPdbFileName,false, "");
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" test1 "<<std::endl;
            //homologJobVector.back().dbConnection->setPdbStatusFail(); // Preemptively set status to FAIL in case the translateMutationVectorFromParent() step fails. 
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            if (homologJobVector.back().updBiopolymerClassContainer().checkAllResidueNumbersAndInsertionCodes()){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" There is a residue numbering issue with this HomologJob. ";
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" and complex string "<<homologJobVector.back().getComplexString()<<"  will be discarded."<<std::endl;
                //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // We have to put this in all outcomes of the if - elseif- else statement. 
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            }
            else if (homologJobVector.back().translateMutationVectorFromParent()){
                //homologJobVector.back().dbConnection->setPdbStatusFail(); // This step can fail, e.g. for 3CYE.pdb. 
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Failed to    translate  mutation vector from parent to HomologJob. ";
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" and complex string "<<homologJobVector.back().getComplexString()<<"  will be discarded."<<std::endl;
                //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // Not sure if we should wait even longer to declare success. here all we are saying is that the coordinate matching worked, which is the usual problem.
                //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // Here we are saying translateMutationVectorFromParent() succeeded.
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            } 
	    else {
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // We have to put this in all outcomes of the if - elseif- else statement. 
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                double myRmsd = 1111.1111;
                bool sequenceAlignmentScoreIsSatisfactory = false;
                bool localSequenceAlignmentScoreIsSatisfactory = false; 
                ParameterReader *myParameterReader = new ParameterReader;
	        // SCF the above trips a seg fault on 16 oct 2018
                myParameterReader->initializeDefaults(std::string(breederParameterReader.singleMutantFilesDirectory + "/parameters.csv").c_str() ); // if no argument is given, initializeDefaults sets leontisWesthofInFileName to  "/parameters.csv".
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->numReportingIntervals = >"<<myParameterReader->numReportingIntervals<<"<"<<std::endl;
                // The below were 90 , 1, and 30  but that was not enough for 3NSS vs 4B7Q or 4B7M.
                myParameterReader->numReportingIntervals = 150; // This was 30, but that was not enough for 2HTY (child) vs. 3NSS (parent). I would go with infinity, but we need some way to cut off ridiculously-expensive jobs
                myParameterReader->reportingInterval     = 10;
                myParameterReader->alignmentForcesForceConstant = 300;
                myParameterReader->detectConvergence = 1; // This will stop the simulation if potential energy < 0.5 kJ/mol
                myParameterReader->convergenceTimeout = 2; // Number of reportingIntervals the potential energy must be stationary to declare convergence	
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->numReportingIntervals = >"<<myParameterReader->numReportingIntervals<<"<"<<std::endl;
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->thermostatType = >"<<myParameterReader->thermostatType<<"<"<<std::endl;
		
                ConstrainedDynamics  myConstrainedDynamics(myParameterReader);
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->thermostatType = >"<<myParameterReader->thermostatType<<"<"<<std::endl;
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<std::endl;
		std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<homologJobVector.back().getCachedPdbId()<<endl;
		std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<homologJobVector.back().dbConnection->getPdbId()<<endl;
		std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<homologJobVector.back().dbConnection->getJobID()<<endl;
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myRmsd = "<<myRmsd<<std::endl;   
                if (homologJobVector.back().getCachedPdbId() == (*(homologJobVector.back().getParentPrimaryJobPointer())).getPdbId()) {
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<std::endl;
                    homologJobVector.back().printData();
                    myRmsd = 0.0; // If this is a self-match, don't bother computing RMSD, it is obviously zero. Also the structurallyAlignOnPrimaryJobAndCalcRmsd doesn't do the structure matching correctly for this case.
                    homologJobVector.back().dbConnection->setRmsd((*(homologJobVector.back().getParentPrimaryJobPointer())).getPdbId()          , homologJobVector.back().getPdbId() ,
                                                (*(homologJobVector.back().getParentPrimaryJobPointer())).getComplexString()  , homologJobVector.back().getComplexString() ,
                                                myRmsd);
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myRmsd = "<<myRmsd<<std::endl;   
                    sequenceAlignmentScoreIsSatisfactory = true; // This can always be said of self-matches.
		    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<homologJobVector.back().getCachedPdbId()<<endl;
  		    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<homologJobVector.back().dbConnection->getPdbId()<<endl;
		    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<homologJobVector.back().dbConnection->getJobID()<<endl;
                } else {
		    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<homologJobVector.back().getCachedPdbId()<<endl;
  		    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<homologJobVector.back().dbConnection->getPdbId()<<endl;
		    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<homologJobVector.back().dbConnection->getJobID()<<endl;
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<std::endl;
                    sequenceAlignmentScoreIsSatisfactory = false; // Just being paranoid
                    //homologJobVector.back().dbConnection->close(); // safer to close the connection before going into a big procedure like this one.
                    // prepareToAlignOnPrimaryJobAndCalcSequenceAlignmentScores returns 0 for satisfactory, 1 for unsatisfactory. 
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->thermostatType = >"<<myParameterReader->thermostatType<<"<"<<std::endl;
                    //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // The method prepareToAlignOnPrimaryJobAndCalcSequenceAlignmentScores only runs if the PDB is not flagged as FAIL. So let's reset to success, and trust that it will exit with FAIL if the following does not work.
                    sequenceAlignmentScoreIsSatisfactory = (!(homologJobVector.back().prepareToAlignOnPrimaryJobAndCalcSequenceAlignmentScores(*myParameterReader,myConstrainedDynamics) ));
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->thermostatType = >"<<myParameterReader->thermostatType<<"<"<<std::endl;
                    //homologJobVector.back().initializeDbConnection((*(homologJobVector.back().getParentPrimaryJobPointer())).breederParameterReader, (*(homologJobVector.back().getParentPrimaryJobPointer())).breederParameterReader.jobId); // Now we are done, need to reopen the connection.
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;

                    localSequenceAlignmentScoreIsSatisfactory = false; 
                    // This does not work yet. Debug later. For now, we harvest current results.  
                    if (0) localSequenceAlignmentScoreIsSatisfactory = (!(homologJobVector.back().calcLocalSequenceAlignment(myParameterReader->atomSpringContainer.getGappedThreadingVector(), myConstrainedDynamics.getCurrentState() )));
                    // For now, we will leave the old behavior hard coded. We need to harvest existing results first.
                    localSequenceAlignmentScoreIsSatisfactory = false; 
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->thermostatType = >"<<myParameterReader->thermostatType<<"<"<<std::endl;
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader->numReportingIntervals = >"<<myParameterReader->numReportingIntervals<<"<"<<std::endl;

                    if (sequenceAlignmentScoreIsSatisfactory || localSequenceAlignmentScoreIsSatisfactory) {
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                        //homologJobVector.back().dbConnection->close(); 
                        //homologJobVector.back().initializeDbConnection((*(homologJobVector.back().getParentPrimaryJobPointer())).breederParameterReader, (*(homologJobVector.back().getParentPrimaryJobPointer())).breederParameterReader.jobId);
                        if (homologJobVector.back().dbConnection->countRmsdEntriesBothWays((*(homologJobVector.back().getParentPrimaryJobPointer())).getPdbId()          , 
                                                                                       homologJobVector.back().getPdbId() ,
                                                                                       (*(homologJobVector.back().getParentPrimaryJobPointer())).getComplexString()  , 
                                                                                       homologJobVector.back().getComplexString()))  {
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myRmsd = "<<myRmsd<<std::endl;   
                            myRmsd = homologJobVector.back().dbConnection->getRmsd((*(homologJobVector.back().getParentPrimaryJobPointer())).getPdbId()          ,
                                                                                       homologJobVector.back().getPdbId() ,
                                                                                       (*(homologJobVector.back().getParentPrimaryJobPointer())).getComplexString()  , 
                                                                                       homologJobVector.back().getComplexString()  );  
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myRmsd = "<<myRmsd<<std::endl;   
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" For HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" , we found an RMSD of "<<myRmsd<<" in the rmsd table. No need to compute this quantity again."<<std::endl;
                   
                        } else {
                            // No RMSD has been computed previously. Makes sense to go ahead and compute RMSD:
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" For HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" , we found NO RMSD in the rmsd table. We will now proceed to compute this quantity."<<std::endl;
                            //dbConnection->setPdbStatusFail();   // Set status to FAIL on presumption of failure of following steps. If we in fact succeed this will be updated to SUCCESS below. 
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.thermostatType = >"<<myParameterReader->thermostatType<<"<"<<std::endl;
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myRmsd = "<<myRmsd<<std::endl;   
                            homologJobVector.back().structurallyAlignOnPrimaryJobAndCalcRmsd (*myParameterReader,myConstrainedDynamics, myRmsd)  ;  
                            //thread t1 (&HomologJob::structurallyAlignOnPrimaryJobAndCalcRmsd  ,&tempHomologJob, (&myParameterReader), myConstrainedDynamics, (myRmsd))  ;
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;   
                            //thread t1 (&HomologJob::writeRenumberedPdbIfAbsent,&tempHomologJob);
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myRmsd = "<<myRmsd<<std::endl;   
                            //t1.join();
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<std::endl;
                            homologJobVector.back().dbConnection->setRmsd((*(homologJobVector.back().getParentPrimaryJobPointer())).getPdbId()          , 
                                                                      homologJobVector.back().getPdbId() ,
                                                                      (*(homologJobVector.back().getParentPrimaryJobPointer())).getComplexString()  , 
                                                                      homologJobVector.back().getComplexString() ,
                                                                      myRmsd);
                        }
            //////
            // Rechecking memory
            //////
            sysinfo (&memInfo);
            totalVirtualMem = memInfo.totalram;
            //Add other values in next statement to avoid int overflow on right hand side...
            totalVirtualMem += memInfo.totalswap;
            totalVirtualMem *= memInfo.mem_unit;
            physMemUsed = memInfo.totalram - memInfo.freeram;
            //Multiply in next statement to avoid int overflow on right hand side...
            physMemUsed *= memInfo.mem_unit;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Physical memory used : "<<physMemUsed<<" bytes "<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Physical memory used : "<<1.0*physMemUsed/(1024*1024*1024)<< " GB "<<std::endl;
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                    } else { // if !sequenceAlignmentScoreIsSatisfactory.  In other words, if sequence alignment is not satisfactory.
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Sequence alignment satisfaction returned "<<sequenceAlignmentScoreIsSatisfactory<<" . Will discard this job a few lines below. RMSD will be left at the default telltale value of "<<myRmsd <<std::endl;
                    }
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<std::endl;
		/*
  */ 
                }
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                double maxRmsd = 0.6;
                if ((myRmsd <= maxRmsd) && sequenceAlignmentScoreIsSatisfactory) {
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" RMSD of "<<myRmsd<<" is acceptable. Will continue with the HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" and complex string "<<homologJobVector.back().getComplexString()<<"  "<<std::endl;   
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                        numMatchingComplexes++;
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Successfully translated mutation vector from parent to HomologJob"<<std::endl;
			std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<homologJobVector.back().getCachedPdbId()<<endl;
			std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<homologJobVector.back().dbConnection->getPdbId()<<endl;
			std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<homologJobVector.back().dbConnection->getJobID()<<endl;
                        // now tempHomologJob's getMutationString() will work from  tempHomologJob's  mutationVector, which is duly translated.
                        if (homologJobVector.back().dbConnection->getNumJobs("completed", homologJobVector.back().getMutationString() )){
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " This job has already been computed. Moving on to the next one."<<std::endl;
               std::string leafJobCompleteEmailContents = "Dear User,\n\n \
\
Thank you for using the homologyScanner web server. Your contribution not only provides you with the DDG for your protein of choice, it also provides you with the PDB IDs of structurally related and possibly useful complexes, and lastly translates your mutation to the numbering system of those other complexes.\n\n\
\
If you are getting this message, we have found a prior existing calculation relevant to your request, for the single PDB structure as named above. There may still be other PDB structures queued, and if so you will receive another email soon.\n\n\
\
If useful, this result has been added to the synopsis table. If the calculation has been done on any other homologs, these are available there as well. If there are any homologs still in the queue, their results will be added to the table when those are completed. The link to the table was sent to you in a prior email. You can also view the results in the structure viewer (click the View tab on biodesign.scilifelab.se , and select the values from the drop down lists.).\n\n Bye" ;
    
               system(("/usr/bin/sendemail -m \"" + leafJobCompleteEmailContents + "\"  -u \"Update: prior computed results relevant to your request were found for PDB ID: " + homologJobVector.back().dbConnection->getPdbId() +"\" -t " + homologJobVector.back().dbConnection->emailAddress + ",samuelfloresc@gmail.com -f sam@pe1.scilifelab.se ").c_str());


                            //continue;
                        } else {
                            // SCF good place to populate sequence table. Then it will be ready for when breeder wants to set the pdbNumberedMutationString.
                            // Except it appears the biopolymers have already been renumbered at this point. so no.
                            //Chromosome tempChromosome (homologJobVector.back(). getMutationString(), homologJobVector.back().updBiopolymerClassContainer() );
                            //tempChromosome.populateSequenceTable(*(homologJobVector.back().dbConnection)); // does this need an asterisk?
                            
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " This job has NOT already been computed. Continuing with it."<<std::endl;
                            // We used to check for failure associated with pdb IDs. However this led to the problem that another job running simultaneously could provisionally set status to FAIL, and the current job would conclude the job is not worth running. However we are now running much faster, so it is not even worth checking this.
	                    /*if (homologJobVector.back().dbConnection->getPdbStatusFail()) {
                                //continue;
	                    } else { */
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                            homologJobVector.back().printBreederCommand();
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
	                    //}
                        }
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                        if (numMatchingComplexes > 1) {mysqlAggregationCommand += std::string(" OR ");}
                        mysqlAggregationCommand += homologJobVector.back().getMysqlAggregationFragment();
                } else {
                    if (!(myRmsd <= maxRmsd)) std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" RMSD was "<<myRmsd<<" , higher than the threshold of "<<maxRmsd<<" nm "<<std::endl; 
                    if (!(sequenceAlignmentScoreIsSatisfactory)) {std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Sequence alignment score was not satisfactory." ;}
                    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" and complex string "<<homologJobVector.back().getComplexString()<<"  will be discarded."<<std::endl;
                }
            };

            //homologJobVector.back().dbConnection->setPdbStatusSuccess(); // moved this line down here since homologJobVector.back().translateMutationVectorFromParent() was failing, e.g. for 3CYE.pdb


            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologJobVector.back().updBiopolymerClassContainer().deleteAllBiopolymerClasses(); // We won't need the BiopolymerClasses anymore, and they take up a lot of memory.
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            homologJobVector.back().dbConnection->close() ;                  


            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
         } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The HomologJob with PDB ID "<<homologJobVector.back().getCachedPdbId()<<" and complex string "<<homologJobVector.back().getComplexString()<<" does NOT have chains corresponding to all those in the parent. We will discard this job."<<std::endl;
         };
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;



        //delete tempHomologJob;


            //////
            // Rechecking memory
            //////
            sysinfo (&memInfo);
            totalVirtualMem = memInfo.totalram;
            //Add other values in next statement to avoid int overflow on right hand side...
            totalVirtualMem += memInfo.totalswap;
            totalVirtualMem *= memInfo.mem_unit;
            physMemUsed = memInfo.totalram - memInfo.freeram;
            //Multiply in next statement to avoid int overflow on right hand side...
            physMemUsed *= memInfo.mem_unit;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Physical memory used : "<<physMemUsed<<" bytes "<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Physical memory used : "<<1.0*physMemUsed/(1024*1024*1024)<< " GB "<<std::endl;


        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        homologJobVector.pop_back(); // Done with HomologJob at end of vector, so get rid of it now. If this was the very last, the while loop will terminate.
    }
    mysqlAggregationCommand += std::string(") and not ISNULL(foldx_energy_wild_type) and not ISNULL(foldx_energy); ");
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" mysqlAggregationCommand = "<<std::endl<<mysqlAggregationCommand<<std::endl;
   
}




std::string modifiedParentChainId( std::string parentChainId){
    //std::string parentChainIdModifier = "P";
    //std::string parentChainIdModifier = "";
    //return (parentChainIdModifier  + parentChainId);
    return (PARENTCHAINPREFIX  + parentChainId);
}
std::string unModifiedParentChainId( std::string modifiedParentChainId){
    //std::string parentChainIdModifier = "P";
    //std::string parentChainIdModifier = "";
    //return (parentChainIdModifier  + parentChainId);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" extracting unmodified parent chain ID from  "<<modifiedParentChainId <<std::endl;
    if (modifiedParentChainId.substr(0,1) != PARENTCHAINPREFIX){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  Error! The prefix of "<<modifiedParentChainId.substr(0,1)<<" was not the expected value of : "<<PARENTCHAINPREFIX<< std::endl; exit(1); 
    }    
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" returning   "<< modifiedParentChainId.substr(1,1) <<std::endl;
    return modifiedParentChainId.substr(1,1);  
}


void HomologJob::sequenceAlignmentIsSatisfactory( bool & needToCompute, bool & isSatisfactory,const double sequenceIdentityCutoff = SEQUENCEIDENTITYCUTOFF ){
    needToCompute = 1; // if the results are unsatisfactory because they have not been computed, then we need to compute now.
    isSatisfactory = 0; // This will trip only if all chains are satisfactory. It is not possible to have isSatisfactory = 1 and needToCompute = 1. 
    int numSatisfactoryChains = 0; 
    // isSatisfactory needToCompute    Truth table:
    // 0              0                Match is known to be unsatisfactory. Do not compute alignment. Return and tell the program this HomologyJob is NOT kosher. 
    // 0              1                Only for NULL or missing entries. They are not satisfactory YET, but need to be computed to give them a chance.
    // 1              0                Cannot happen.   If known to be satisfactory, need to recompute. See below.
    // 1              1                If match is already known to be satisfactory, you might think there is no need to recompute. However actually we do, in order to get the simulation system set up for later RMSD calculation. 

    // This was incorrect:    
    // 1              0                If match is already known to be satisfactory, no need to recompute. Tell the program that this HomologJob is kosher as far as sequence alignment is concerned.
    // 1              1                Cannot happen. If it is known to be satisfactory, there is no need to recompute.
    for (auto it=primaryToHomologChainIdMap.begin(); it!=primaryToHomologChainIdMap.end(); ++it){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Primary job chain :"<< it->first <<" has corresponding homolog job chain :  "<< it->second << " . " << std::endl;
        std::string primaryChain = it->first ;
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        std::string homologChain = it->second;
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        ((*getParentPrimaryJobPointer()).getPdbId());
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        std::string myParentChainId = ((*getParentPrimaryJobPointer()).getPdbId());
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        std::string myHomologChainId = getPdbId();
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        //int numSatisfactoryChains = 0;

	    if (dbConnection->countEntriesInMatchingChains( 
                    myParentChainId, myHomologChainId,
                    //((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
		    primaryChain,
		    homologChain))
                { // If there is an entry:
                    bool mySequenceIdentityIsNull = 0;
	            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
		    if (dbConnection->matchingChainsValueIsNull(
			((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
			primaryChain, 
			homologChain,
                        "sequenceIdentity"
		    ))
                    { // continue with the procedure. NULL here means the sequenceIdentity needs to be computed.
			std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sequenceIdentity is NULL. So we will need to continue with the calculation. Continue to that now.."<<std::endl; 
                        needToCompute = 1;
                        // do not increment numSatisfactoryChains    
                        isSatisfactory = 0;
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" needToCompute = "<<needToCompute<<" , isSatisfactory = "<<isSatisfactory<<std::endl;
                        return ;   
                        // No need to check the rest of the chains. If even one chain needs to be computed, we will continue the calculation. Obviously NULL is not "satisfactory".
		    } else {
                        bool dummyBool; // ACtually we already issued matchingChainsValueIsNull above, so no need to check for nulls here.
	                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
			if ( dbConnection->chainSequenceAlignmentIsSatisfactory( 
			    ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
			    primaryChain, 
			    homologChain, 
			    90.0 )
			)
                        {
			    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sequenceIdentity has been computed and is NOT NULL.     It is      found to be   satisfactory. However there may still be more chains to check.."<<std::endl;
                            //isSatisfactory = 1;
                            numSatisfactoryChains ++; // increment number of satisfactory chains.
                            // Do not return yet.  Have to make sure ALL chains are satisfactory.
			} else { // chainSequenceAlignmentIsSatisfactory returned FALSE, so UNsatisfactory sequenceIdentity
			    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sequenceIdentity has been computed and is NOT NULL. But it is also found to be UNsatisfactory. Aborting calculation. Tell the program not to compute alignment."<<std::endl;
                            needToCompute = 0; // 
                            // do not increment numSatisfactoryChains    
                            isSatisfactory = 0; 
                            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" needToCompute = "<<needToCompute<<" , isSatisfactory = "<<isSatisfactory<<std::endl;
			    return ;
                            // If result is UNsatisfactory for even one chain, then we abort. No need to go through the rest of this procedure.
			}
		    } // in this block matchingChainsValueIsNull is FALSE
		} else { // countEntriesInMatchingChains returned 0, so no entries:
			std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sequenceIdentity has NOT been computed .  Continuing procedure , we need to compute this quantity now."<<std::endl;
                        isSatisfactory = 0;
                        needToCompute  = 1; 
                        // do not increment numSatisfactoryChains    
                        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" needToCompute = "<<needToCompute<<" , isSatisfactory = "<<isSatisfactory<<std::endl;
                        return ;
		} // of if countEntriesInMatchingChains

    }
    // At this point, all chains are satisfactory. Just confirm that the right number are satisfactory:
    if (numSatisfactoryChains == primaryToHomologChainIdMap.size()){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" numSatisfactoryChains = "<<numSatisfactoryChains<<" , primaryToHomologChainIdMap.size() = "<< primaryToHomologChainIdMap.size()<<std::endl;
        isSatisfactory = 1;
        needToCompute  = 1; 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" needToCompute = "<<needToCompute<<" , isSatisfactory = "<<isSatisfactory<<std::endl;
        return ; 
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" numSatisfactoryChains = "<<numSatisfactoryChains<<" , primaryToHomologChainIdMap.size() = "<< primaryToHomologChainIdMap.size()<<" . Unexplained error!"<<std::endl;
        exit(1);
    } 


}

// This moves the HomologJob's entire BiopolymerClassContainer to superimpose on that of the PrimaryJob. The alignment is based only on the correspondences indicated in the  primaryToHomologChainIdMap
// Returns 0 if sequence alignment was acceptable, 1 otherwise.
int    HomologJob::prepareToAlignOnPrimaryJobAndCalcSequenceAlignmentScores (ParameterReader & myParameterReader ,  ConstrainedDynamics &  myConstrainedDynamics) {  
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    bool isSatisfactory = 0; 
    bool needToCompute  = 1;
    double sequenceIdentityCutoff = SEQUENCEIDENTITYCUTOFF;
    sequenceAlignmentIsSatisfactory(needToCompute, isSatisfactory, SEQUENCEIDENTITYCUTOFF);   
    if (!(needToCompute)){
        if (isSatisfactory) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" This HomologJob is known to be satisfactory in terms of sequence alignment. This means we need to compute, in order to set up the simulation system  "<<std::endl; exit(1);
        } else { 
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" This HomologJob is known to be UNsatisfactory in terms of sequence alignment.  "<<std::endl;
        }
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Apparently we do not need to compute the sequence alignement. Continuing with the rest of the program now."<<std::endl;
        return (!(isSatisfactory)); // sequenceAlignmentIsSatisfactory has the opposite convention from prepareToAlignOnPrimaryJobAndCalcSequenceAlignmentScores in terms of this return value.
    } else {
        if (isSatisfactory) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" This HomologJob is known to be satisfactory in terms of sequence alignment. So why are we going on with the sequence alignment ? To set up the simulation system. "<<std::endl;
            //exit(1);
        } else { 
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" This HomologJob is of unknown sequence alignment status. So we will now compute sequence alignment.  "<<std::endl;
        }
    }

    // really HomologJob's biopolymerClassContainer should itself just be a member of a formal parameterReader member.
    // Now we will copy the BiopolymerClass's from the Parent PrimaryJob into the HomologJob's myParameterReader's biopolymerClassContainer
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // Do we need some sort of preinitialization?
    //myParameterReader.initializeDefaults(std::string(getParentPrimaryJobPointer()->breederParameterReader.singleMutantFilesDirectory + "/parameters.csv").c_str() ); // if no argument is given, initializeDefaults sets leontisWesthofInFileName to  "/parameters.csv".
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " myParameterReader.leontisWesthofInFileName = "<<myParameterReader.leontisWesthofInFileName <<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    myParameterReader.firstStage = 2;
    myParameterReader.lastStage =2;
    // I was using this before. However it was a problem because if the dynamics completed but for some reason we did not manage to delete the parent chains and write to  getAlignedStructureFileName(), we wound up with a structure that had parent chains.  
    // So now we explicitly create a separate file name for this:
    myParameterReader.lastFrameFileName =  dbConnection->getWorkingDirectory() + std::string("/last.pdb"); 
    myParameterReader.readPreviousFrameFile = 0; // not sure if this is right eitehr
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " myParameterReader.leontisWesthofInFileName = "<<myParameterReader.leontisWesthofInFileName <<std::endl;
    // Reporting intervals of 10 ps take a long time, the first one over a couple of minutes. Probably need on the order of 100 of them.
    //myParameterReader.reportingInterval = 20.000; // debugging. change. later.
    //myParameterReader.numReportingIntervals = 9; // debugging. change. later. // defaults to 90

    // Get BiopolymerClass's from current HomologJob
    myParameterReader.myBiopolymerClassContainer = updBiopolymerClassContainer();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just copied  myParameterReader.myBiopolymerClassContainer = updBiopolymerClassContainer(), now checking transforms:"<<std::endl;
    myParameterReader.myBiopolymerClassContainer.printTopLevelTransforms();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done  checking transforms"<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" done checking transforms:"<<std::endl;
    myParameterReader.myBiopolymerClassContainer.constrainAllChainsToEachOther(myParameterReader.constraintToGroundContainer);  // All chains in HomologJob will be constrained to each other. We have not yet loaded the parent chains yet, so those are immune to this . // Turns out the minimizer does not work well with constraints. So are not using constraints.

    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // This will load all the parent's BiopolymerClasses in the HomologJob's biopolymerClassContainer EXCEPT that the chain IDs will all have PARENTCHAINPREFIX prepended to avoid conflict    
    myParameterReader.myBiopolymerClassContainer.loadSequencesFromPdb( (*getParentPrimaryJobPointer()).getPdbNumberedStructureFileName() ,false, PARENTCHAINPREFIX,false,true); 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::map <std::string, int> tempParentChainAndComplexNumberMap = (*getParentPrimaryJobPointer()).getChainAndComplexNumberMap();
    for (auto parentIterator =  tempParentChainAndComplexNumberMap.begin(); parentIterator != tempParentChainAndComplexNumberMap.end(); parentIterator++){
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
       std::string chainIdInParent = parentIterator->first ;
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Working on chain : "<<parentIterator->first<<"  of complex "<< parentIterator->second <<std::endl;
       myParameterReader.myBiopolymerClassContainer.updBiopolymerClass(modifiedParentChainId(chainIdInParent)).setFirstResidueMobilizerType(std::string("Weld") ); // All chains in ParentJob will be fixed to ground.
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    } 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    myParameterReader.mobilizerContainer.setMobilizerTypeForAllChains(String("Rigid"), myParameterReader.myBiopolymerClassContainer);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::string correspondingPrimaryChainsString = "";
    std::string correspondingHomologChainsString = "";
    for (auto it=primaryToHomologChainIdMap.begin(); it!=primaryToHomologChainIdMap.end(); ++it){
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Primary job chain :"<< it->first <<" has corresponding homolog job chain :  "<< it->second << " . " << std::endl;
       correspondingPrimaryChainsString += it->first;
       correspondingHomologChainsString += it->second;
       ThreadingStruct thread;
       std::string alignmentChainInParent = it->first;
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
       // SCF
       // 0 is HomologJob, 1 is PrimaryJob
       thread.updThreadingPartner(0).biopolymerClass =  myParameterReader.myBiopolymerClassContainer.updBiopolymerClass(getCorrespondingChain(alignmentChainInParent));
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
       thread.forceConstant   = myParameterReader.alignmentForcesForceConstant;
       thread.updThreadingPartner(1).biopolymerClass =  myParameterReader.myBiopolymerClassContainer.updBiopolymerClass(modifiedParentChainId(alignmentChainInParent));
       thread.setDefaultStartEndResidues(); // sets the start and end residues to the BiopolymerClass FirstResidue and LastResidue, for the two BiopolymerClass's. Note that the BiopolymerClass's must be defined first.
       //thread.isGapped        = myParameterReader.alignmentForcesIsGapped;
       thread.deadLengthIsFractionOfInitialLength = myParameterReader.alignmentForcesDeadLengthIsFractionOfInitialLength;
       if (thread.deadLengthIsFractionOfInitialLength){std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Unexplained error!"<<std::endl; exit(1); }
       thread.deadLengthFraction = myParameterReader.alignmentForcesDeadLengthFraction;
       if (thread.deadLengthFraction > 0) {std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Unexplained error!"<<std::endl; exit(1); }
       // enforce thread.updThreadingPartner(0).biopolymerClass to be from the HomologJob
       if ((thread.updThreadingPartner(0).biopolymerClass.getChainID() ).compare(it->second) != 0){std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Unexplained error!"<<std::endl; exit(1);}
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.atomSpringContainer.getGappedThreadingVector().size() = "<< myParameterReader.atomSpringContainer.getGappedThreadingVector().size()<<std::endl;
       myParameterReader.atomSpringContainer.addGappedThreading(thread, myParameterReader.myBiopolymerClassContainer );
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.atomSpringContainer.getGappedThreadingVector().size() = "<< myParameterReader.atomSpringContainer.getGappedThreadingVector().size()<<std::endl;
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Primary job chain :"<< it->first <<" has corresponding homolog job chain :  "<< it->second << " . Just created a gappedThreading between these two chains. " << std::endl;
       cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<endl;
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Transform for chain "<<modifiedParentChainId(alignmentChainInParent)<<": "<<std::endl;
       myParameterReader.myBiopolymerClassContainer.updBiopolymerClass(modifiedParentChainId(alignmentChainInParent) ).printTopLevelTransform() ;//<<"<"<<std::endl;
       std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Transform for chain "<<getCorrespondingChain(alignmentChainInParent)<<":"<<std::endl;
       myParameterReader.myBiopolymerClassContainer.updBiopolymerClass(getCorrespondingChain(alignmentChainInParent) ).printTopLevelTransform() ;//<< std::endl;
   

    } // of for

    myParameterReader.postInitialize(); // this might be needed in order to process  myParameterReader.mobilizerContainer and maybe  myParameterReader.atomSpringContainer
    myParameterReader.atomSpringContainer.printAllAlignmentStats();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.atomSpringContainer.getGappedThreadingVector().size() = "<< myParameterReader.atomSpringContainer.getGappedThreadingVector().size()<<std::endl;
    myParameterReader.outTrajectoryFileName = getWorkingDirectory() + "/trajectory.align-homolog-on-primary." + correspondingPrimaryChainsString + "-" + correspondingHomologChainsString + ".pdb";
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::remove(myParameterReader.outTrajectoryFileName); // Have to delete the trajectory file prior to use
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    myParameterReader.printAllSettings(std::cout,String("") );
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;

    myConstrainedDynamics.initializeDumm();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;

    // The below replaces the first part of runDynamics()
    //initializeDbConnection((*getParentPrimaryJobPointer()).breederParameterReader , (*getParentPrimaryJobPointer()).breederParameterReader.jobId  );//,getCachedPdbId());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    setPdbId(getPdbId());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<getCachedPdbId()<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<dbConnection->getPdbId()<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<dbConnection->getJobID()<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // It is not necessary to check this here. We would not have gotten to this point if this were a failed job, taht is detected higher up in printCorrespondenceTable
    //if (dbConnection->getPdbStatusFail()){
        //dbConnection->close();
    //    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" A previous unsuccessful attempt was made to read in this PDB. In the future we will look for an alternate structure. For now we just discard this HomologJob.."<<std::endl;
    //    return 1;
    //}    

    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; 
    //dbConnection->setPdbStatusFail();   // Set status to FAIL on presumption of failure of following steps. If we in fact succeed this will be updated to SUCCESS below. 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;

    if (myConstrainedDynamics.initializeBiopolymersAndCustomMolecules()) {
        //dbConnection->close();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" initializeBiopolymersAndCustomMolecules failed. Probably your input structure has a problem. Abort this job. " << std::endl;
        return 1; // The sequence aligment is too distant for our purposes.
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" initializeBiopolymersAndCustomMolecules succeeded. Continue with this job. " << std::endl;
    }    
 
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just ran initializeBiopolymersAndCustomMolecules , now checking transforms:"<<std::endl;
    myParameterReader.myBiopolymerClassContainer.printTopLevelTransforms();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done  checking transforms"<<std::endl;

    //dbConnection->setPdbStatusSuccess(); // Not sure if we should wait even longer to declare success. here all we are saying is that the coordinate matching worked, which is the usual problem.

    cout<<__FILE__<<":"<<__LINE__<<endl;
    myConstrainedDynamics.initializeBodies();
    cout<<__FILE__<<":"<<__LINE__<<endl;
    myConstrainedDynamics.initializeCustomForcesConstraints(); // This calls createSpringsFromGappedThreading


    cout<<__FILE__<<":"<<__LINE__<<endl;



    for (int i = 0; i < myParameterReader.atomSpringContainer.getGappedThreadingVector().size() ; i++){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].printAlignmentStats();
        seqan::AlignmentStats myAlignmentStats = (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].getAlignmentStats();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        double myScore =  (myAlignmentStats.numMatches + myAlignmentStats.numMismatches);
        if  (myScore > 0) myScore = ( myAlignmentStats.numMatches) / myScore;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" We may in the future wish to use the metric: ( myAlignmentStats.numMatches) / (myAlignmentStats.numMatches + myAlignmentStats.numMismatches) = " << myScore <<std::endl;
        // Per convention partner 0 is the homologJob, partner 1 is the PrimaryJob
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()) = "<< unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()) <<std::endl;    
        dbConnection->setMatchingChainsValue( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
            unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
            (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            std::string("matches"), myAlignmentStats.numMatches); 
        dbConnection->setMatchingChainsValue( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
            unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
            (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            std::string("mismatches"), myAlignmentStats.numMismatches); 
        dbConnection->setMatchingChainsValue( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
            unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
            (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            std::string("insertions"), myAlignmentStats.numInsertions); 
        dbConnection->setMatchingChainsValue( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
            unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
            (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            std::string("deletions"), myAlignmentStats.numDeletions); 
        // I am computing sequence identity as total matches divided by length of PARENT chain. This means the homolog could be gigantically longer, but not gigantically shorter, and still get a high sequenceIdentity. Test this wiht A-106-E.3CL2.E,B.leaf-job.log, which got 32.6425 for sequence identity, with 
        //double mySequenceIdentity = myAlignmentStats.numMatches / (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainLength();
        //dbConnection->setMatchingChainsValue( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
        //    unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
        //    (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
        //    std::string("sequenceIdentity"), mySequenceIdentity ); 
        // Below is the old way of setting sequenceIdentity. Problem was that it was based on the entire alignment, and the chains could be very different lengths. So alignment could be perfect over the aligned region, and sequenceIdentity could still be very low.
        dbConnection->setMatchingChainsValue( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
            unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
            (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            std::string("sequenceIdentity"), myAlignmentStats.alignmentIdentity); 
       if ( dbConnection->chainSequenceAlignmentIsSatisfactory( ((*getParentPrimaryJobPointer()).getPdbId())  ,  getPdbId(), 
            unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()),  
            (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),    
            SEQUENCEIDENTITYCUTOFF)
            ){

            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The alignment scores were satisfactory. Continue with this job. " << std::endl;
            // do NOT break or return yet. We have to check all the other chains and make sure they are kosher as well.
        } else {
            if (dbConnection->matchingChainsValueIsNull((*getParentPrimaryJobPointer()).getPdbId(), getPdbId(),unModifiedParentChainId((myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(1).biopolymerClass.getChainID()), (myParameterReader.atomSpringContainer.getGappedThreadingVector())[i].updThreadingPartner(0).biopolymerClass.getChainID(),"sequenceIdentity")){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The alignment scores were NULL for this chain. Continue with this job. " << std::endl;
           
            } else {
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" The alignment scores were UNsatisfactory. Abort this job. " << std::endl;
                //dbConnection->close();
                return 1; // The sequence aligment is too distant for our purposes.
            }
        }     
    } 
    //
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    //dbConnection->close();
    return 0;
}

double HomologJob::structurallyAlignOnPrimaryJobAndCalcRmsd (ParameterReader & myParameterReader, ConstrainedDynamics &  myConstrainedDynamics , double & myRmsd ) {  
    // Leave this to be checked in printCorrespondenceTable
    /*if (dbConnection->getPdbStatusFail()){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" dbConnection->getPdbStatusFail() for this PDB ID returned: "<<dbConnection->getPdbStatusFail()<<" . Thus we will abort this function."<<std::endl;
        return myRmsd;
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" dbConnection->getPdbStatusFail() for this PDB ID returned: "<<dbConnection->getPdbStatusFail()<<" . Thus we will continue this function."<<std::endl;
        dbConnection->setPdbStatusFail();   // Set status to FAIL on presumption of failure of following steps. If we in fact succeed this will be updated to SUCCESS below. 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.thermostatType = >"<<myParameterReader.thermostatType<<"<"<<std::endl;
    }*/
    // The below replaces the second part of runDynamics():  
    

    // within runDynamics, here is the second part of initializeDynamics() :
    cout<<__FILE__<<":"<<__LINE__<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.thermostatType = >"<<myParameterReader.thermostatType<<"<"<<std::endl;
    myConstrainedDynamics.createEventHandlers(); // This should not be needed if all we do is minimize
    cout<<__FILE__<<":"<<__LINE__<<endl;
    cout<<__FILE__<<":"<<__LINE__<<endl;
    myConstrainedDynamics.initializeIntegrator();              // This calls the minimizer, if that is on.
    cout<<__FILE__<<":"<<__LINE__<<endl;
    //cout<<__FILE__<<":"<<__LINE__<<" Net charge over "<<_dumm.getNumIncludedAtoms ()<<" atoms included in physics zone = "<< _dumm.getTotalIncludedCharge()<<endl;
    // end replacement of initializeDynamics() .
    cout<<__FILE__<<":"<<__LINE__<<endl;
    myRmsd = myParameterReader.atomSpringContainer.calcKabschRmsd(myConstrainedDynamics.getCurrentState(), myParameterReader.myBiopolymerClassContainer);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myParameterReader.atomSpringContainer.calcKabschRmsd(myConstrainedDynamics.getCurrentState(), myParameterReader.myBiopolymerClassContainer) = "<<myRmsd<<std::endl;
    // SCF As of 16 Apr 2019, the following is no longer needed. We will compute using calcKabschRmsd!
    /*
    myConstrainedDynamics.runAllSteps();
    cout<<__FILE__<<":"<<__LINE__<<endl;
    myConstrainedDynamics.postDynamics();
    */
    cout<<__FILE__<<":"<<__LINE__<<endl;
    // end replace runDynamics()

    //myConstrainedDynamics.runDynamics();
    //double 
    // SCF As of 16 Apr 2019, the following is no longer needed. We will compute using calcKabschRmsd!
    //myRmsd = myParameterReader.atomSpringContainer.calcRmsd(myConstrainedDynamics.getCurrentState(), myParameterReader.myBiopolymerClassContainer);
    
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Got RMSD = "<<myRmsd<<std::endl;
    myRmsd = myParameterReader.atomSpringContainer.calcKabschRmsd(myConstrainedDynamics.getCurrentState(), myParameterReader.myBiopolymerClassContainer);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" After runAllSteps, myParameterReader.atomSpringContainer.calcKabschRmsd(myConstrainedDynamics.getCurrentState(), myParameterReader.myBiopolymerClassContainer) = "<<myRmsd<<std::endl;


    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
    for (int i = 0 ; i < myParameterReader.myBiopolymerClassContainer.getNumBiopolymers(); i++) {
        // Check all chains in myParameterReader.myBiopolymerClassContainer
        String myChainID = myParameterReader.myBiopolymerClassContainer.updBiopolymerClass(i).getChainID();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Checking chain "<< myChainID  <<std::endl;
        // Check whether current HomologJob's BiopolymerClassContainer has the chain. If it does not have it, then it came from the parent and should be deleted.
        if (!(updBiopolymerClassContainer().hasChainID(myChainID))) {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Decided to delete chain "<< myChainID <<" since it must be from the parent"<<std::endl;
            myParameterReader.myBiopolymerClassContainer.deleteBiopolymerClass(myChainID); // Delete a chain that must be from the parent.
            i--; // myParameterReader.myBiopolymerClassContainer.getNumBiopolymers() would now return one less, so we have to be sure not to miss any chains.
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Decided NOT to delete chain "<< myChainID <<" since it must be from the current HomologJob"<<std::endl;
        }
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
    } // of for

    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
    std::ofstream fullOutputStream(std::string(getAlignedStructureFileName() + ".full.pdb").c_str());
    myParameterReader.myBiopolymerClassContainer.writeDefaultPdb(  fullOutputStream);
    cout<<__FILE__<<":"<<__LINE__<<endl;
    std::ofstream outputStream(getAlignedStructureFileName().c_str());
    
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to write "<<getAlignedStructureFileName()<<" with the parent biopolymers removed."<<std::endl;
    myParameterReader.myBiopolymerClassContainer.writePdb(myConstrainedDynamics.getCurrentState(),myConstrainedDynamics.getCompoundSystem(),  outputStream);
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    // if not done here, The following will now need to be done outside the method

    setBiopolymerClassContainer ( myParameterReader.myBiopolymerClassContainer ) ; // The HomologJob's complex should now be displaced to superimpose with parent's
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;

    // if not done here, The following will now need to be done outside the method
    //dbConnection->setPdbStatusSuccess();   // Set status toI FAIL on presumption of failure of following steps. If we in fact succeed this will be updated to SUCCESS below. 
    /*
    */
    // SCF As of 16 Apr 2019, the following is no longer needed. We will compute using calcKabschRmsd!
    return myRmsd;
    //return myKabschRmsd;
}

int HomologJob::translateMutationVectorFromParent(){
    // std::map <std::string , std::string> primaryToHomologChainIdMap;
    if (updBiopolymerClassContainer().updMutationVector().size() > 0){
         MMBLOG_FILE_FUNC_LINE(INFO,": mutationVector has :"<<updBiopolymerClassContainer().updMutationVector().size()<<" elements. Expected 0! Quitting now to be safe. "<<std::endl);
         //ErrorManager::instance <<__FILE__<<":"<<__LINE__<<": mutationVector has :"<<updBiopolymerClassContainer().updMutationVector().size()<<" elements. Expected 0! Quitting now to be safe. "<<std::endl;
         //ErrorManager::instance.treatError();
    }
    updBiopolymerClassContainer().updMutationVector().clear(); // just for extra safety
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;  
    BiopolymerClassContainer *parentBiopolymerClassContainer = new BiopolymerClassContainer();
    *parentBiopolymerClassContainer =  (*getParentPrimaryJobPointer()).updBiopolymerClassContainer();
    //BiopolymerClassContainer parentBiopolymerClassContainer =   (*getParentPrimaryJobPointer()).getBiopolymerClassContainer();
    vector <Mutation> parentMutationVector = (*parentBiopolymerClassContainer).getMutationVector();
    //vector <Mutation> parentMutationVector = parentBiopolymerClassContainer.getMutationVector();
    int success = 0; // assume failure. This will be actively set to 1 in teh case of success.
    for (int i = 0; i < parentMutationVector.size() ; i++){
    //for (int i = 0; i < (*getParentPrimaryJobPointer()).updBiopolymerClassContainer().getMutationVector().size() ; i++){
	//(*getParentPrimaryJobPointer()).updBiopolymerClassContainer().updMutationVector()[i].print();    
	parentMutationVector[i].print();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;  
        std::string parentChain = parentMutationVector[i].getChain();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" parentMutationVector[i].getChain() = "<<parentMutationVector[i].getChain()<<std::endl;  
      
        std::string myChain = (primaryToHomologChainIdMap.find(parentChain))->second; // Retrieve the chain in HomologJob corresponding to parentChain in parent job
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" myChain = "<<myChain<<std::endl;  
        BiopolymerClass myBiopolymerClass = updBiopolymerClassContainer().updBiopolymerClass(myChain); // If this step fails, it is possible that the HomologJob is using a PDB ID that has had that chain manually removed. fasta goes straight to RCSB, rather than using the manipulated structures.  So you can remove the chain from the fasta file if you really don't want to use it.
        //BiopolymerClass parentBiopolymerClass = parentBiopolymerClassContainer.updBiopolymerClass(parentChain);
        //TAlign align = updBiopolymerClassContainer().updBiopolymerClass(myChain).createGappedAlignment( (*parentBiopolymerClassContainer).updBiopolymerClass(parentChain) ); // The second argument , gap penalty, has a default value of -1
	
	//New way to create alignments. MMB no longer uses BiopolymerClassContainer, instead one must create a ThreadingStruct using an AtomSpringContainer, and then use the ThreadingStruct's computeAlign() method.
	AtomSpringContainer myAtomSpringContainer;       	
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"calling myAtomSpringContainer.createGappedThreading( "<<myChain<<","<<parentChain<<","<<10000<<","<<1<<",updBiopolymerClassContainer()  , (*parentBiopolymerClassContainer))"<<  std::endl;  
	// setting spring constant to 10k, then backboneONly to true 
        ThreadingStruct myThreadingStruct = myAtomSpringContainer.createGappedThreading(myChain , parentChain,10000 , 1 ,updBiopolymerClassContainer()  , (*parentBiopolymerClassContainer));
        //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;  
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<myThreadingStruct.updThreadingPartner(0).biopolymerClass. getSequence()<<std::endl;
	std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<myThreadingStruct.updThreadingPartner(1).biopolymerClass. getSequence()<<std::endl;
        myThreadingStruct.setLongSequences();
        TAlign align = myThreadingStruct.computeAlign();
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;  

        //TAlign align = updBiopolymerClassContainer().updBiopolymerClass(myChain).createGappedAlignment( parentBiopolymerClass ); // The second argument , gap penalty, has a default value of -1
        //bool successfullyFoundCorrespondingMutationInCurrentBiopolymer = false;
        Mutation myMutation  ;
        if (myBiopolymerClass.getCorrespondingMutationInCurrentBiopolymer((*parentBiopolymerClassContainer).updBiopolymerClass(parentChain),align,parentMutationVector[i],myMutation) ){ // Return value of 0 indicate success
        //if (myBiopolymerClass.getCorrespondingMutationInCurrentBiopolymer(parentBiopolymerClass,align,parentMutationVector[i],myMutation) ){ // Return value of 0 indicate success
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Failed to translate : "<<std::endl;
            parentMutationVector[i].print();
            return 1; // nonzero return value indicates failure.
             
        } else {
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Successfully translated "<<endl;
            parentMutationVector[i].print();
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" to "<<endl;
            myMutation.print();
        }
        updBiopolymerClassContainer(). addMutationToVector(myMutation);
	/*
	*/
    }
    return 0 ; // Return value of 0 indicates success.
}
;

bool HomologJob::complexMatchesParent(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    (*getParentPrimaryJobPointer()).printData();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Parent's chainAndComplexNumberMap is of size :"<<  (*getParentPrimaryJobPointer()).getChainAndComplexNumberMap().size() <<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Parent's complex string is : "<<  (*getParentPrimaryJobPointer()).getComplexString() <<std::endl;
    std::string tempChildComplexString = getComplexString() ;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Child's complex string is : "<<   tempChildComplexString<<std::endl;
    bool allParentChainsHaveUniqueCorrespondingChildChain = true ;
    // have been warned to turn auto into std::unique_ptr cuz the former is deprecated
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" (*getParentPrimaryJobPointer()).getChainAndComplexNumberMap().size() = " <<(*getParentPrimaryJobPointer()).getChainAndComplexNumberMap().size()<<std::endl;    
    
    std::map <std::string, int> tempParentChainAndComplexNumberMap = (*getParentPrimaryJobPointer()).getChainAndComplexNumberMap();
    for (auto parentIterator = tempParentChainAndComplexNumberMap.begin(); parentIterator != tempParentChainAndComplexNumberMap.end(); ++parentIterator){
        std::string chainIdInParent = parentIterator->first ;
        int complexNumberInParent   = parentIterator->second;
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Chain ID in parent : "<< chainIdInParent  << " with complex number: "<< complexNumberInParent  << " has : "<<std::endl;
        // For some reason had trouble working with calls to  getPrimaryToHomologChainIdMap() . Made local copy:
        std::map <std::string , std::string> myPrimaryToHomologChainIdMap = getPrimaryToHomologChainIdMap();
         
        auto childIterator = myPrimaryToHomologChainIdMap.find(chainIdInParent); //begin();
        //auto childIterator = getPrimaryToHomologChainIdMap().find(chainIdInParent); //begin();
        //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        if (childIterator == myPrimaryToHomologChainIdMap.end()) {
            //std::cout<<std::endl;
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            std::cout<<" NO corresponding chain in child. Returning false."<<std::endl; 
            allParentChainsHaveUniqueCorrespondingChildChain = false;
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            return allParentChainsHaveUniqueCorrespondingChildChain;
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        } else {
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            std::cout<<" corresponding chain "<<childIterator->second<<" in child. "<<std::endl; 
            allParentChainsHaveUniqueCorrespondingChildChain = true;
        }
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
        
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" All parent chains have corresponding chains in the child. So far, no reason to reject. Let's check for multiple child chains having same corresponding parent chain. Anyway, for now allParentChainsHaveUniqueCorrespondingChildChain =  "<<allParentChainsHaveUniqueCorrespondingChildChain<<std::endl;

    return allParentChainsHaveUniqueCorrespondingChildChain;

} // of complexMatchesParent

std::string HomologJob::getComplexString(){
    std::string myComplexString = ","; // the complex string will in the end look like AB,CD where A and B are in complex 0, and C and D are in complex 1. So we start with the "," in the middle. Then we add chains from complex 0 on the left side, and chains from complex 1 on the right.  Order doesn't matter much here, except maybe aesthetically.
    //printData();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;  
    for (auto it=primaryToHomologChainIdMap.begin(); it!=primaryToHomologChainIdMap.end(); ++it){
        int complexNumberInParent = -2222;
        std::string chainIdInHomologJob = it->second ; // This is the chain ID in the current job, which is of type HomologJob.
        std::string chainIdInParentJob = it->first  ; // This is the chain ID in the parent  job, which is of type PrimaryJob.
        complexNumberInParent = ((*getParentPrimaryJobPointer()).getChainAndComplexNumberMap().find( chainIdInParentJob ))->second; //  we chainIdInParentJob to go into the parent's chainAndComplexNumberMap, and retrieve the complex number (->second). Complex numbers are stored in the parent PrimaryJob, not in the HomologJob .
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
        if (complexNumberInParent == 0){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to modify complex string : "<<myComplexString<<" . "<<std::endl;
            //myComplexString = chainIdInHomologJob + myComplexString;
            // maps are sorted by key. So we create the complex strings from left to right and they should be in alphabetica order within each complex.   
            insertBeforeComma(myComplexString,chainIdInHomologJob);    
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just added chain "<<chainIdInHomologJob<< " of complex "<< complexNumberInParent <<" on the left, albeit rightmost within complex. Chain ID in parent was "<< chainIdInParentJob  <<" . Complex string is now : "<<myComplexString<<" . "<<std::endl;
        }
        else if (complexNumberInParent == 1){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
            //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to modify complex string : "<<myComplexString<<" . "<<std::endl;
            myComplexString = myComplexString + chainIdInHomologJob ;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just added chain "<<chainIdInHomologJob<< " of complex "<< complexNumberInParent <<" on the right. Chain ID in parent was "<< chainIdInParentJob<<" . Complex string is now : "<<myComplexString<<" . "<<std::endl;
        }
        else { std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found that chain "<<chainIdInHomologJob<<" belongs to complex "<< complexNumberInParent <<" . Chain ID in parent was "<< chainIdInParentJob<<" . Complex numbers are restricted to 0 and 1!"<<std::endl; exit(1);}
    }
    if (myComplexString.length() != (1+primaryToHomologChainIdMap.size())){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  complex string : "<<myComplexString<<" has length "<< myComplexString.length()<<" . This does not match the number of elements in primaryToHomologChainIdMap : "<< primaryToHomologChainIdMap.size()<<std::endl; exit (1);
    } else {
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  complex string : "<<myComplexString<<" has length "<< myComplexString.length()<<" . This DOES match the number of elements in primaryToHomologChainIdMap : "<< primaryToHomologChainIdMap.size()<<std::endl;
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Returning complex string : "<<myComplexString<<" . For reference, the complex string in the parent is : "<< (*getParentPrimaryJobPointer()).getComplexString()  <<""<<std::endl;
    return myComplexString;
};



std::string HomologJob::getMutationString(){
   return updBiopolymerClassContainer().getFormattedMutationsString(std::string("-"));  
}; 

void  HomologJob::printData(){
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" I am a HomologJob with following characteristics: "<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" PDB ID (cached): "<<getCachedPdbId()<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" PDB ID (mysql) : "<<dbConnection->getPdbId()<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Complex string "<<getComplexString()<<std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" primaryToHomologChainIdMap: "<<std::endl;
    //SCF fix this
    for (auto it=primaryToHomologChainIdMap.begin(); it!=primaryToHomologChainIdMap.end(); ++it){
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Primary job chain :"<< it->first <<" has corresponding homolog job chain :  "<< it->second << " . " << std::endl;
    } // of for
} // of printData()

void  HomologJob::setParentPrimaryJobPointer(PrimaryJob * myPrimaryJob){
    parentPrimaryJobPointer = myPrimaryJob;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Currently inside HomologJob. "<<std::endl;// with following characteristics: "<<std::endl;
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Printing  HomologJob data: "<<std::endl;// with following characteristics: "<<std::endl;
    //printData();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<":"<<__FUNCTION__<<" Printing  PrimaryJob data: "<<std::endl;// with following characteristics: "<<std::endl;
    getParentPrimaryJobPointer()->printData();
}

std::string HomologJob:: getMysqlAggregationFragment(){
    std::string mysqlAggregationFragment= std::string(" (pdbId   = \"") + getPdbId() + std::string("\" and mutationString = \"") + getMutationString() + std::string("\" ) ");
    //std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" mysqlPossiblyUsefulCommand = "<< std::endl <<mysqlPossiblyUsefulCommand<<std::endl;;
    return mysqlAggregationFragment;
    
}
std::string PrimaryJob::commonBreederParameters(){
    std::string returnString = std::string(" -FASTAEXECUTABLE ")
    + breederParameterReader.fastaExecutable
    + std::string(" -FASTATEMPDIRECTORY ")   + breederParameterReader.fastaTempDirectory
    + std::string(" -BREEDEREXECUTABLE ")    + breederParameterReader.breederExecutable 
    + std::string(" -BREEDERMAINDIRECTORY ") + breederParameterReader.breederMainDirectory 
    + std::string(" -DATABASE ")             + breederParameterReader.database //mmb  
    + std::string(" -MMBEXECUTABLE ")        + breederParameterReader.MMBExecutable 
    + std::string(" -LASTSTAGE ")            + std::to_string(breederParameterReader.lastStage)
    + std::string(" -FOLDXSCRIPT ")          + breederParameterReader.foldXScript 
    + std::string(" -FOLDXEXECUTABLE ")      + breederParameterReader.foldXExecutable 
    + std::string(" -SQLEXECUTABLE  ")       + breederParameterReader.SQLExecutable
    + std::string(" -ACCOUNT ")              + breederParameterReader.account // SLURM account
    + std::string(" -USER ")                 + breederParameterReader.user
    + std::string(" -SQLSYSTEM MySQL ")   
    + std::string(" -SQLSERVER ")            + breederParameterReader.server //rembrandt.bmc.uu.se
    + std::string(" -SQLPASSWORD ")          + breederParameterReader.password //    MMB37xYstRhggY78432  
    + std::string(" -SQLUSER ")              + breederParameterReader.user //  mmbadmin 
    + std::string(" -JOBLIBRARYPATH ")       + breederParameterReader.jobLibraryPath 
    + std::string(" -MOBILIZERRADIUS ")      + std::to_string (breederParameterReader.mobilizerWithinRadius) // 0.0 
    + std::string(" -REPORTINGINTERVAL ")    + std::to_string (breederParameterReader.reportingInterval) //  5.0 
    + std::string(" -NUMREPORTINGINTERVALS ")+ std::to_string (breederParameterReader.numReportingIntervals) //  2  
    + std::string(" -ID ")                   +  dbConnection->getJobID()
    + std::string(" -TEMPERATURE ")          +  std::to_string(breederParameterReader.temperature) 
    + std::string(" -EMAILADDRESS ")         +  breederParameterReader.emailAddress; 
    return returnString;
}

void  HomologJob::printBreederCommand(){
    // Aurora defaults:
    //std::string sqlExecutable = "/sw/easybuild/software/Compiler/GCC/4.9.3-binutils-2.25/MySQL/5.6.26-clientonly/bin/mysql";
    //std::string workingDirectory = std::string("/lunarc/nobackup/users/samuelf/") + getPdbId() + "/";
    //std::string slurmAccount = "snic2015-16-49";
    // Uppmax defaults:
    std::string sqlExecutable = getParentPrimaryJobPointer()->breederParameterReader.SQLExecutable; 
    std::string slurmAccount  = getParentPrimaryJobPointer()->breederParameterReader.account;
    setWorkingDirectory();
    //std::string workingDirectory =  (*getParentPrimaryJobPointer()).workingDirectory + std::string("/") + getPdbId() + "/";
    std::string breederExecutable =  getParentPrimaryJobPointer()->breederParameterReader.breederExecutable;
    
    std::string part1 = breederExecutable + getParentPrimaryJobPointer()->commonBreederParameters()
        + std::string(" -PDBID ")                +  getPdbId()
        + std::string(" -WTRENUMBEREDSTRUCTUREFILENAME ")  +  getRenumberedPdbFileName();   

    updBiopolymerClassContainer().setOriginalSequencesFromCurrentSequences();
    std::string part2 = "  -ONEMUTANT " + updBiopolymerClassContainer().getFoldxFormattedMutations(); 
    std::string part3 = " -WORKINGDIRECTORY " + getWorkingDirectory();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl;
    std::string part4 = "";
    // above was getWorkingDirectory() + getPdbId() + ".pdb";
    // -LASTSTAGE 1 forces MMB to die and just do a FoldX-only run    
    std::string part5 = std::string(" -CHAINSINCOMPLEX ") + getComplexString();
    std::string part6 = std::string(" &> ") + getWorkingDirectory() + "/" +  getJobName() + ".breeder.out"; //std::string part5 = " -LASTSTAGE " + std::to_string(getParentPrimaryJobPointer()->breederParameterReader.lastStage) + " -CHAINSINCOMPLEX " + getComplexString();
    //std::string part5 = " -RAMP 1 -CHAINSINCOMPLEX " + getComplexString();
    std::string breederCommand = part1+part2+part3+part4+part5 + part6;
    printData();
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob cached PDB ID: "<<getCachedPdbId()<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob mysql  PDB ID: "<<dbConnection->getPdbId()<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" HomologJob jobName      : "<<dbConnection->getJobID()<<endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" "<<std::endl<<breederCommand<<std::endl;
    // setSubmittedHomologs takes args:  std::string pdbPrimary, std::string pdbHomolog , std::string complexPrimary, std::string complexHomolog, std::string mutationStringPrimary,  std::string mutationStringHomolog  
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  getParentPrimaryJobPointer()->getMutationStringFromBiopolymerClassContainer() : " <<  getParentPrimaryJobPointer()->getMutationStringFromBiopolymerClassContainer() << std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  getParentPrimaryJobPointer()->getMutationString() : " <<  getParentPrimaryJobPointer()->getMutationString() << std::endl;
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"  getMutationString() :   "<<  getMutationString()<<std::endl;
    dbConnection->setSubmittedHomologs( getParentPrimaryJobPointer()->getPdbId() , getPdbId(),  getParentPrimaryJobPointer()->getComplexString(),  getComplexString(),  getParentPrimaryJobPointer()->getMutationStringFromBiopolymerClassContainer(),  getMutationString());
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" breederCommand.c_str() = "<<breederCommand.c_str()<<std::endl;
    //system(breederCommand.c_str()); 
    // Changed this to use SLURM instead:
    submitCommandtoSlurm(breederCommand.c_str(), std::string(getJobName() + std::string(".breeder")));
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
}

// We don't want to merge two primaryToHomologChainIdMaps which have the same chains, either in parent or child. This routine tests that. Returns true if such redundancy is found.
bool primaryToHomologChainIdMapsAreOverlapping (std::map <std::string , std::string> & map1, std::map <std::string , std::string> & map2){
    
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
    for (auto it1=map1.begin(); it1!=map1.end(); ++it1){ 
        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1->first, it1->second = "<<it1->first<<", "<<it1->second<<std::endl;
   
        for (auto it2=map2.begin(); it2!=map2.end(); ++it2){ 
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it2->first, it2->second = "<<it2->first<<", "<<it2->second<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1->first, it2->first = "<<it1->first<<", "<<it2->first<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" it1->second, it2->second = "<<it1->second<<", "<<it2->second<<std::endl;
            if (it1->first == it2->first){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found redundancy, it1->first == it2->first , "<<it1->first<<" == "<<it2->first<<std::endl;
                return 1;
            }
            if (it1->second == it2->second){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found redundancy, it1->second == it2->second , "<<it1->second<<" == "<<it2->second<<std::endl;
                return 1;
            }
        }
    }
    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found NO redundancy"<<std::endl;
    return 0;
}

void HomologJob::mergePrimaryToHomologChainIdMaps(std::map <std::string , std::string> mapToInsert) {
            if (primaryToHomologChainIdMapsAreOverlapping(primaryToHomologChainIdMap, mapToInsert)) {
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found redundancy in map to be inserted. I Refuse to proceed!"<<std::endl;
                return;
            } else {
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Found NO redundancy. I will proceed to merge the map as ordered, kind sir / madam."<<std::endl;
            }
            int expectedSize = primaryToHomologChainIdMap.size() + mapToInsert.size();
            primaryToHomologChainIdMap.insert(mapToInsert.begin(),mapToInsert.end());
            if (primaryToHomologChainIdMap.size() != expectedSize){std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Error! Size of expanded primaryToHomologChainIdMap = "<<primaryToHomologChainIdMap.size()<<" , was expecting : "<<expectedSize<<std::endl;  exit(1);}
            else {std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Size of expanded primaryToHomologChainIdMap = "<<primaryToHomologChainIdMap.size()<<" ,  expected : "<<expectedSize<<" . All is fine! "<< std::endl;}
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Contents of the expanded primaryToHomologChainIdMap are:"<<std::endl;
            for (auto it=primaryToHomologChainIdMap.begin(); it!=primaryToHomologChainIdMap.end(); ++it){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Primary job chain :"<< it->first <<" has corresponding homolog job chain :  "<< it->second << " . " << std::endl;
            } // of for
        }  


