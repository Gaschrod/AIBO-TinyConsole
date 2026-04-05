#ifndef PortConfig_h_DEFINED
#define PortConfig_h_DEFINED

#include <vector>
using std::vector;
#include <OPENR/OPENR.h>  // oserviceNAME_MAX

const int MAX_HOSTNAME = 64;

struct ServicePort {
    char serviceName[oserviceNAME_MAX];
    unsigned long port;
    char hostname[MAX_HOSTNAME];

#if 0
    ServicePort() {}
    ServicePort(const ServicePort& s){
        strcpy(serviceName, s.serviceName);
        strcpy(hostname, s.hostname);
        port = s.port;
    }
#endif
};

class PortConfig {

public:
    PortConfig();
    ~PortConfig();

    int LoadConfiguration(const char* filename);

    int NumOfSubject()  const { return nSubject_; }
    int NumOfObserver() const { return nObserver_; }

    char* ServiceName(int index) { return registry_[index].serviceName; }
    char* Hostname(int index)    { return registry_[index].hostname; }
    unsigned long Port(int index){ return registry_[index].port; }

    unsigned long GetProxyPort()     const { return proxy_port_; }
    char*         GetProxyHostname() { return proxy_host_; }

private:
    int  AddServicePort(char* line);
    void RegisterConfiguration(char* line);

    vector<ServicePort>  registry_;
    int nSubject_;
    int nObserver_;

    // For configuration
    unsigned long proxy_port_;
    char          proxy_host_[MAX_HOSTNAME];
    char          default_host_[MAX_HOSTNAME];

    static const char* const WhiteSpace;   /* space or tab */
    static const char        CommentMark;  /* line ignored */
    static const char        ConfigMark;   /* DNS configuration line */
    static const char* const Separator;    /* Service name separator */
};

#endif
