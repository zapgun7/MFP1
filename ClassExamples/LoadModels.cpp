#include "cMesh.h"
#include <vector>

extern std::vector< cMesh* > g_vec_pMeshesToDraw;


bool LoadModels(void)
{
    cMesh* pPolyMesh = new cMesh();
    pPolyMesh->meshName = "somepoly.ply";
    //pGridGroundMesh->bIsWireframe = true;
    //pPolyMesh->bDoNotLight = true;
    // note this does NOT have a physProps, so is ignored by the physics update loop
    pPolyMesh->drawPosition.y = 0.0f;   //  0,-10,0
    pPolyMesh->drawPosition.x = 0.0f;
    pPolyMesh->drawPosition.z = 0.0f;
    pPolyMesh->scale = 0.6f;
    pPolyMesh->friendlyName = "thePoly";
   ::g_vec_pMeshesToDraw.push_back(pPolyMesh);

   cMesh* pPolyMeshWF = new cMesh();
   pPolyMeshWF->meshName = "somepoly.ply";
   pPolyMeshWF->bIsWireframe = true;
   //pPolyMesh->bDoNotLight = true;
   // note this does NOT have a physProps, so is ignored by the physics update loop
   pPolyMeshWF->drawPosition.y = 0.0f;   //  0,-10,0
   pPolyMeshWF->drawPosition.x = 0.0f;
   pPolyMeshWF->drawPosition.z = 0.0f;
   pPolyMeshWF->scale = 0.605f;
   pPolyMeshWF->friendlyName = "thePoly";
   ::g_vec_pMeshesToDraw.push_back(pPolyMeshWF);


    return true;
}
