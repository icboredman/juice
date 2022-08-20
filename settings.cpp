#include <iostream> // cin, cout
#include <string>
#include <stdio.h>  //fopen, fwrite, fread

#include "settings.hpp"


namespace Settings
{
    //-----------------------------------------------
    // Global variables
    //-----------------------------------------------
    #define STORAGE "/home/fish/babelfish_settings.bin"
    t_cluster sCluster;


    t_cluster* Cluster()
    {
        return & sCluster;
    }


    int RetrieveCluster()
    {
        sCluster = {};  // initialize all members to zero

        FILE* fin = fopen(STORAGE, "r");
        if (fin == NULL)
        {
            return -1;
        }
        if (1 != fread(&sCluster, sizeof(t_cluster), 1, fin))
        {
            fclose(fin);
            return -2;
        }
        fclose(fin);
        return 0;
    }

}
