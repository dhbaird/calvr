#ifndef CVR_COLLABORATIVE_MANAGER_H
#define CVR_COLLABORATIVE_MANAGER_H

#include <util/CVRSocket.h>

#include <string>
#include <map>
#include <vector>
#include <queue>

#include <osg/MatrixTransform>

namespace cvr
{

/*struct ClientUpdate
{
        float headPos[3];
        float headRot[4];
        float handPos[3];
        float handRot[4];
        float objScale;
        float objTrans[16];
        int numMes;
};*/

// TODO: reduced struct when changes are done, for now leave for things to compile

struct ClientUpdate
{
    float objScale;
    float objTrans[16];
    int numMes;
};

struct BodyUpdate
{
        float pos[3];
        float rot[4];
};

enum CollabMode
{
    LOCKED,
    UNLOCKED
};

enum CollaborativeMessageType
{
    ADD_CLIENT = 0,
    REMOVE_CLIENT,
    SET_MASTER_ID,
    SET_COLLAB_MODE,
    PLUGIN_MESSAGE
};

struct ServerUpdate
{
        int numUsers;
        CollabMode mode;
        int masterID;
        int numMes;
};

// TODO: add version numbers for server/client interface
struct ServerInitInfo
{
        int numUsers;
};

struct CollaborativeMessageHeader
{
        int type;
        int pluginMessageType;
        char target[256];
        int size;
        bool deleteData;
};

struct ClientInitInfo
{
        int id; ///< server fills this in
        char name[256];
        int numHeads;
        int numHands;
};

class CollaborativeThread;

class CollaborativeManager
{
    public:
        ~CollaborativeManager();

        static CollaborativeManager * instance();

        bool init();

        bool isConnected();

        bool connect(std::string host, int port);
        void disconnect();

        void update();

        int getID() { return _id; }
        const std::string & getName() { return _myName; }
        CollabMode getMode() { return _mode; }
        void setMode(CollabMode mode);
        int getMasterID() { return _masterID; }
        void setMasterID(int id);

        std::map<int,ClientInitInfo> & getClientInitMap() { return _clientInitMap; }
        int getClientNumHeads(int id);
        int getClientNumHands(int id);
        const osg::Matrix & getClientHeadMat(int id, int head);
        const osg::Matrix & getClientHandMat(int id, int hand);

        void sendCollaborativeMessageAsync(std::string target, int type, char * data, int size, bool sendLocal = false);
        void sendCollaborativeMessageSync(std::string target, int type, char * data, int size, bool sendLocal = false);

    protected:
        CollaborativeManager();
        void updateCollabNodes();
        osg::Node * makeHand(int num);
        osg::Node * makeHead(int num);
        void startUpdate();
        void processMessage(CollaborativeMessageHeader & cmh, char * data);

        static CollaborativeManager * _myPtr;

        std::string _myName;

        cvr::CVRSocket * _socket;

        cvr::CollaborativeThread * _thread;

        std::map<int,ClientUpdate> _clientMap;
        std::map<int,ClientInitInfo> _clientInitMap;

        int _id;
        bool _connected;
        int _masterID;
        CollabMode _mode;

        std::map<int,std::vector<BodyUpdate> > _handBodyMap;
        std::map<int,std::vector<BodyUpdate> > _headBodyMap;

        osg::ref_ptr<osg::MatrixTransform> _collabRoot;
        std::map<int,std::vector<osg::ref_ptr<osg::MatrixTransform> > > _collabHands;
        std::map<int,std::vector<osg::ref_ptr<osg::MatrixTransform> > > _collabHeads;

        std::queue<std::pair<CollaborativeMessageHeader,char*> > _messageQueue;
};

}

#endif
