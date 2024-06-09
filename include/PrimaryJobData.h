#include <string>
#include  <vector>
#include "BiopolymerClass.h"
#include "ParameterReader.h"
#include "BreederParameterReader.h"
#include "Repel.h"
// This is needed for smart pointers (including unique_ptr)"               
#include <memory>




class GenericJob {

    private:
        //std::string pdbId; // PDB ID of  job
        std::string mutationString; // mutation string of  job
        BiopolymerClassContainer biopolymerClassContainer; // Not sure if we need this. But could come in handy if we need to disambiguate on a structural basis.
        //std::string structureFileName;
    protected: //Allows child classes to access, but no one outside the class can access.
        //std::string workingDirectory; // This is the directory in which this object's work will be done. It should include the structureFileName , and one subdirectory for each mutant.
    public:
        //unique_ptr<DBManager> dbConnection(new DBManager);
        //DBManager * dbConnection = malloc(sizeof(DBManager)) ;
        //DBManager * dbConnection = (DBManager*) malloc(sizeof(DBManager)) ;
        DBManager * dbConnection ;
        //DBManager * dbConnection = new DBManager;
        //void        setStructureFileName();//std::string);
        int checkFileStatus(std::string fileName);
        virtual std::string getDefaultPdbNumberedStructureFileName(); //getWorkingDirectory() + getPdbId() + std::string(".pdb");
        virtual std::string getPdbNumberedStructureFileName();// tries getAlternateStructureFileName() first, then getWorkingDirectory() + getPdbId() + std::string(".pdb")
        //virtual void initializeSequencesFromPdb(){
        //    initializeSequencesFromPdb( getPdbNumberedStructureFileName());
        //}; //second parameter is proteinCapping .. going with the usual default, though we should reexamine it.
        void setMutationVectorFromString();
        const BiopolymerClassContainer getBiopolymerClassContainer(){return biopolymerClassContainer;} 
        BiopolymerClassContainer &     updBiopolymerClassContainer(){return biopolymerClassContainer;} 
        void setBiopolymerClassContainer(BiopolymerClassContainer & myBiopolymerClassContainer){biopolymerClassContainer = myBiopolymerClassContainer;}
        std::string getWorkingDirectory(){return dbConnection->getWorkingDirectory();}// workingDirectory;}
        std::string getSequence(std::string myChain){return biopolymerClassContainer.updBiopolymerClass(myChain).getSequence();};
        virtual void printData() =0; // This is how we define pure virtual functions.
        virtual std::string getMutationString() =0;
        virtual std::string getComplexString() =0;
        virtual std::string getPdbId();//{return dbConnection->getPdbId();}; 
        //void setPdbId(std::string myPdbId){ pdbId = myPdbId;};
        std::string getCorrespondingChain(std::string chainInPrimaryJob);
        void initializeDbConnection ( BreederParameterReader &  myBreederParameterReader , std::string myJobId ){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;    
            dbConnection = new MysqlConnection(myBreederParameterReader. server ,     myBreederParameterReader.password ,  myBreederParameterReader. user ,  myBreederParameterReader. database );
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;    
            
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" calling dbConnection->setPdbId("<<myBreederParameterReader.pdbId<<")"<<std::endl;    
            dbConnection->setPdbId(myBreederParameterReader.pdbId );
	    std::string tempPdbId = dbConnection->getPdbId();
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" calling dbConnection->getPdbId() returns: >"<< tempPdbId<<"< "<<std::endl;    
            dbConnection->setJobID(myBreederParameterReader.jobId );
            //dbConnection->setJobID(myBreederParameterReader.pdbId);
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Done initializing dbConnection  for job:"<<std::endl;
            printData();
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;    
        }
        GenericJob(){ mutationString = "";   
            //dbConnection =   (DBManager*) malloc(sizeof(DBManager)) ; 
            //dbConnection = new DBManager;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(biopolymerClassContainer) = "<<sizeof(biopolymerClassContainer)<<std::endl;
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" sizeof(*dbConnection) = "<<sizeof(*dbConnection)<<std::endl;
        }
        ~GenericJob(){ 
            //free(dbConnection);
            //delete dbConnection;
            //dbConnection = NULL;
            updBiopolymerClassContainer().deleteAllBiopolymerClasses(); // Not sure this is really necessary
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" Just destroyed a GenericJob (may have been a PrimaryJob or HomologJob)"<<std::endl; ;   
        }
        void fetchPdb();// This issues mkdir workingDirectory , then fetches pdbId from rcsb.org.  To-Do: override this if the user specifies a structureFileName .
        void validate();
};

class HomologJob ; // Forward declaration

class PrimaryJob : public  GenericJob{ // This is the main job. It may be that we treat this as a figurehead, with no heavy calculations. One of the homolog jobs could correspond to the primary job, without any need to treat it as a special case
    
    private:
        //using GenericJob::initializeSequencesFromPdb;
        std::map <std::string, int> chainAndComplexNumberMap; // the key is the chain ID in the primary job  , the mapped value is the complex number, either 0 or 1.      
        std::vector <HomologJob> homologJobVector;
        std::string mutationString;
        bool hasBeenCloned;
        std::string cachedWorkingDirectory;
	//ParameterReader myParameterReader;
    public:
        void setHasBeenCloned(bool myHasBeenCloned){hasBeenCloned = myHasBeenCloned;}
        bool getHasBeenCloned(){return hasBeenCloned ;}
        PrimaryJob(){ mutationString = ""; setHasBeenCloned(0);}  
        BreederParameterReader breederParameterReader;   
        std::string overarchingDirectory;
        //std::string jobLibraryPath  ;
        std::map <std::string, int> getChainAndComplexNumberMap (){return chainAndComplexNumberMap;};
        std::string getPdbId();
        void printData();
        void setWorkingDirectory();//{workingDirectory = overarchingDirectory + std::string("/") + getPdbId() + std::string("/");}; // This method sets the working directory to the overarching directory +"/" + current object's pdbId .
        void setCachedWorkingDirectory(){cachedWorkingDirectory = overarchingDirectory + std::string("/") + getPdbId() + std::string("/");}; // This method sets the working directory to the overarching directory +"/" + current object's pdbId .
        std::string getCachedWorkingDirectory(){std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl; return cachedWorkingDirectory;};
        std::string getMutationString();
        std::string getMutationStringFromBiopolymerClassContainer();
        std::string getComplexString();
        void loadChainAndComplexNumberMap(std::string complexString); // loads chainAndComplexNumberMap. chains before the "," assigned to complex 0, after "," assigned to complex 1
        void printBreederCommands(); 
        void listChains();
        void setMutationString(std::string myMutationString){mutationString = myMutationString;};
        void setMutationVectorFromMutationString();
        bool homologJobHasChainsWithUnsatisfactorySequenceIdentity( HomologJob & homologJob,  bool sequenceIdentityIsNull,  double mySequenceIdentityCutoff );
        void loadHomologJobVectorFromFasta(std::string myChain);
        void addHomologJob(HomologJob & myHomologyJob); // this procedure should first check that the job does not already exist.  If it does exist, do not add redundant. Also check to see if maybe merging jobs is appropriate. If so, decide which job to merge to.
        void loadHomologJobVector(){
	    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            printData();	    
	    std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            for (auto it=chainAndComplexNumberMap.begin(); it!=chainAndComplexNumberMap.end(); ++it){
	        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                std::string myChainId = it->first;
	        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
	        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" loadHomologJobVectorFromFasta("<<myChainId<<") "<<std::endl;
	        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
                loadHomologJobVectorFromFasta(myChainId);
	        std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;
            } // of for auto it
        };
        void printCorrespondenceTable();
        void initializeSequencesFromPdb(std::string myPdbFileName); // This one automatically prepends PARENTCHAINPREFIX to all chain IDs.
        void initializeSequencesFromPdb(){   
            std::string tempStructureFileName = getPdbNumberedStructureFileName();
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" About to call initializeSequencesFromPdb("<<tempStructureFileName<<") "<<std::endl;
	    if (tempStructureFileName == ""){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<" You are trying to call an argument-ful initializeSequencesFromPdb with an empty string as argument! Something is wrong."<<std::endl;
	    exit(1);}
            initializeSequencesFromPdb( getPdbNumberedStructureFileName());
        }; //second parameter is proteinCapping .. going with the usual default, though we should reexamine it.
        std::string commonBreederParameters();
        void spawnSingleHomologyScannerRunsFromHomologJobVector(); // This take homologJobVector and spawns each of its elements as a single homologyScanner run, using a system call
        //void checkSingleHomologAndSpawnBreeder(HomologJob & myHomologJob);
        int  createSingleHomologJobAndAddToVector( std::string myComplexChains, std::string myHomologPdbId);
        //void checkSingleHomologAndSpawnBreeder(HomologJob & myHomologJob);
}; // of GenericJobData PrimaryJob

class  HomologJob : public GenericJob {

    private:
        std::string cachedPdbId; // PDB ID of  job
        std::map <std::string , std::string> primaryToHomologChainIdMap; // the key is the chain ID of the primary job  (which spawned us), the mapped value is the chain ID of the current homolog job.
        PrimaryJob * parentPrimaryJobPointer;
       
    public:
        void printPrimaryToHomologChainIdMap(){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " Cached PDB ID = >"<< getCachedPdbId() << "< "<<std::endl;
            for (auto it=  primaryToHomologChainIdMap.begin(); it!=  primaryToHomologChainIdMap.end(); ++it){
                std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<< " entry in primaryToHomologChainIdMap : >"<< it->first <<"< , >" << it->second <<"< "<<std::endl;
            }
        }  
        std::string getJobName(); 
        std::string getRenumberedPdbFileName();
        int  writeRenumberedPdbIfAbsent();
        void setCachedPdbId(std::string myPdbId){ cachedPdbId = myPdbId;};
        HomologJob(){  setCachedPdbId ( "");}
        void copyChainsFromParent() ; 

        std::string getCorrespondingChain(std::string chainInPrimaryJob) ;
        void setCorrespondingChain(std::string chainInPrimaryJob, std::string chainInHomologJob);
        std::string getCachedPdbId(){ return cachedPdbId ;};
        std::string getPdbId();
        void        setPdbId(std::string); 
        std::string chooseStructureFileName();
        std::string getAlignedStructureFileName();
        std::string getAlignmentTrajectoryFileName();
        bool calcLocalSequenceAlignment(vector<ThreadingStruct> threadingStructVector, State state);
        void sequenceAlignmentIsSatisfactory( bool & needToCompute, bool & isSatisfactory,const double sequenceIdentityCutoff  );
        int    prepareToAlignOnPrimaryJobAndCalcSequenceAlignmentScores (ParameterReader & myParameterReader, ConstrainedDynamics &  myConstrainedDynamics) ;
        double structurallyAlignOnPrimaryJobAndCalcRmsd (ParameterReader & myParameterReader,  ConstrainedDynamics &  myConstrainedDynamics, double & myRmsd) ; 
        int  translateMutationVectorFromParent();
        std::string getMysqlAggregationFragment();
        void setWorkingDirectory(){std:string myWorkingDirectory = (*getParentPrimaryJobPointer()).overarchingDirectory + std::string("/") + getCachedPdbId() + std::string("/");
            dbConnection->setWorkingDirectory(myWorkingDirectory);}; // This method sets the working directory to the parent's overarching directory +"/" + current object's pdbId .
        bool complexMatchesParent();
        const std::map <std::string , std::string> getPrimaryToHomologChainIdMap(){return primaryToHomologChainIdMap;};
        void printData();
        std::string getMutationString();
        std::string getComplexString();
        void setParentPrimaryJobPointer(PrimaryJob * myPrimaryJob);
        void printBreederCommand();
        //std::map <std::string , std::string> getPrimaryToHomologChainIdMap(){return primaryToHomologChainIdMap;}
        void mergePrimaryToHomologChainIdMaps(std::map <std::string , std::string> mapToInsert) ;
        PrimaryJob * getParentPrimaryJobPointer(){return parentPrimaryJobPointer;};
        void initializeSequencesFromPdb(std::string myPdbFileName); // This one automatically prepends PARENTCHAINPREFIX to all chain IDs.
        void initializeSequencesFromPdb(){
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;      
            initializeSequencesFromPdb( chooseStructureFileName());
            std::cout<<__FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<std::endl;      
        }; //second parameter is proteinCapping .. going with the usual default, though we should reexamine it.
        void validate();
        void submitCommandtoSlurm( std::string commandString , std::string jobName);
};


