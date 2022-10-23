#include <unistd.h>

namespace Settings
{
    typedef struct
    {
        uint32_t _dummy_1;
        uint32_t _dummy_2;
        uint32_t _dummy_3;
        uint16_t unit_id;
        //other stuff here...
    } t_cluster;

    t_cluster* Cluster(void);

    int RetrieveCluster(void);
}
