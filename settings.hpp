#include <unistd.h>

namespace Settings
{
    typedef struct
    {
        uint16_t unitID;
    } t_cluster;

    t_cluster* Cluster(void);

    int RetrieveCluster(void);
}
