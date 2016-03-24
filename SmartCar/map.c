#include "map.h"

static int _roadMapSize = 0;
static NodeStruct* _roadMap;

int parseMapFile(char const* fileName)
{
    FILE* mapFile;
    char filePath[128] = "./";
    char line[256];
    int mapSize = -1;
    int i = 0;

    //File located in program work directory
    strcat(filePath, fileName);

    //Read mapfile
    mapFile = fopen(filePath, "r");

    if(mapFile == NULL)
    {
        printf("Could not open file: %s\n", fileName);

        return 1;   //Could not read file
    }

    //Free memory of previous map if existing
    clearRouteMap(_roadMap);

    //Read as many lines as you can
    while(!feof(mapFile))
    {
        if(mapSize == -1)
        {
            fgets(line, sizeof(line), mapFile);
            char* subString;
            subString = strstr(line, "SIZE:");
            if(subString != NULL)
            {
                sscanf(subString, "SIZE: %d,", &mapSize);
            }
        }
        else
        {
            _roadMap = (NodeStruct *) malloc (sizeof(NodeStruct)*mapSize);
            processNodes(mapFile, _roadMap);
            i++;
        }
    }

    //Close mapfile
    fclose(mapFile);

    return 0;
}

int clearRouteMap(NodeStruct* roadMap)
{
    NodeStruct* nodePointer;

    if(roadMap != NULL)
    {
        while((NodeStruct*) roadMap->Next != NULL)
        {
            nodePointer = (NodeStruct*) roadMap->Next;

            while((NodeStruct*) nodePointer->Next != NULL)
            {
                nodePointer = (NodeStruct*) nodePointer->Next;
            }

            //Free allocated memory of last node in list
            free((NodeStruct*) roadMap->Next);
        }

        //Free allocated memory of first node in list
        free(roadMap);

        return 0;
    }

    return 1;
}

int processNodes(FILE* mapFile, NodeStruct* roadMap)
{
    int id, n, o, z ,w, nn, oo, zz, ww;
    id = -2;
    n = -2;
    o = -2;
    z = -2;
    w = -2;
    nn = -2;
    oo = -2;
    zz = -2;
    ww = -2;
    bool rfidFound = false;
    char rfid[21];
    char line[256];

    while(!feof(mapFile))
    {
        fgets(line, sizeof(line), mapFile);

        char* subString1 = NULL;
        char* subString2 = NULL;
        char* subString3 = NULL;
        char* subString4 = NULL;

        subString1 = strstr(line, "ID:");
        subString2 = strstr(line, "NEIGHBOURS:");
        subString3 = strstr(line, "DISTANCE:");
        subString4 = strstr(line, "RFID:");

        //Read ID
        if(subString1 != NULL)
        {
            sscanf(line, "%*s %d,", &id);
            subString1 = NULL;
        }

        //Read n o z w
        if(subString2 != NULL)
        {
            fgets(line, sizeof(line), mapFile); //Get rid of {
            n = processDubiousLine(mapFile);
            o = processDubiousLine(mapFile);
            z = processDubiousLine(mapFile);
            w = processDubiousLine(mapFile);
            subString2 = NULL;
        }

        //Read nn oo zz ww
        if(subString3 != NULL)
        {
            fgets(line, sizeof(line), mapFile);
            nn = processDubiousLine(mapFile);
            oo = processDubiousLine(mapFile);
            zz = processDubiousLine(mapFile);
            ww = processDubiousLine(mapFile);
            subString3 = NULL;
        }

        //Read RFID
        if(subString4 != NULL)
        {
            char* ptr;
            ptr = strpbrk(line, "\"");
            strncpy(rfid, ptr+1, 20);
            rfid[20] = '\0';    //Add terminator string character '\0'
            subString4 = NULL;
            rfidFound = true;
        }

        if(id != -2 && n != -2 && o != -2 && z != -2 && w != -2 && nn != -2 && oo != -2 && zz != -2 && ww != -2 && rfidFound)
        {
            InitMap(roadMap, id, n, o, z, w,  nn, oo, zz, ww, rfid);
            id = -2;
            n = -2;
            o = -2;
            z = -2;
            w = -2;
            nn = -2;
            oo = -2;
            zz = -2;
            ww = -2;
            rfidFound = false;
        }
    }

    return 0;
}

int processDubiousLine(FILE* mapFile)
{
    int j;
    char line[256];

    fgets(line, sizeof(line), mapFile); //Read Values

    if(strstr(line, "NON") != NULL)
    {
        j = NON;
    }
    else if(strstr(line, "INF") != NULL)
    {
        j = INF;
    }
    else
    {
        sscanf(line, "%d%*s", &j);
    }

    return j;
}

void InitMap(NodeStruct* Map, int node, int n,  int o, int z, int w, int nn, int oo, int zz, int ww, char* RFID)
{
	Map[node].Neighbours[0] = n;
	Map[node].Neighbours[1] = o;
	Map[node].Neighbours[2] = z;
	Map[node].Neighbours[3] = w;

	Map[node].Distance[0] = nn;
	Map[node].Distance[1] = oo;
	Map[node].Distance[2] = zz;
	Map[node].Distance[3] = ww;

	Map[node].Visited = 0;

	Map[node].DV = 120;
	Map[node].Previous = -1;
	Map[node].Next = -1;
	Map[node].NextAbsDir = -1;
	Map[node].NextRelDir = -1;

	strncpy(Map[node].RFID, RFID, 21);
}

NodeStruct* getRoadMap(void)
{
    return _roadMap;
}

int getRoadMapSize(void)
{
    return _roadMapSize;
}
