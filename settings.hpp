#include <unistd.h>

namespace Settings
{
    typedef struct
    {
        uint16_t unit_id;
        //other stuff here...
    } t_cluster;

    t_cluster* Cluster(void);

    int RetrieveCluster(void);
}
