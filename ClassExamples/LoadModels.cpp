#include "cMesh.h"
#include <vector>

extern std::vector< cMesh* > g_vec_pMeshesToDraw;

//float getRandomFloat(float a, float b);

bool LoadModels(void)
{
//
////    // Smart array of cMesh object
////    std::vector<cMesh> vecMeshesToDraw;
//    g_vecMeshesToDraw.push_back(bunny1);
//    g_vecMeshesToDraw.push_back(bunny2);
//    g_vecMeshesToDraw.push_back(bathtub);
//    g_vecMeshesToDraw.push_back(terrain);

//     cMesh* pTerrain = new cMesh();
//     pTerrain->meshName = "Terrain_xyz_n_rgba.ply";
//     //bunny2.position = glm::vec3(1.0f, 0.0f, 0.0f);
//     pTerrain->scale = 1.0f;
//     pTerrain->drawPosition.y = -25.0f;
//     ::g_vec_pMeshesToDraw.push_back(pTerrain);
// 
// 
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

    const float MAX_SPHERE_LOCATION = 30.0f;
    const float MAX_VELOCITY = 1.0f;

    // Make a bunch of spheres...
//     const unsigned int NUMBER_OF_SPHERES = 50;
//     for (unsigned int count = 0; count != NUMBER_OF_SPHERES; count++)
//     {
//         cMesh* pSphereMesh = new cMesh();
//         pSphereMesh->meshName = "Sphere_1_unit_Radius.ply";
//         //pSphereMesh->bIsWireframe = true;
//         //pSphereMesh->bDoNotLight = true;
// 
//         pSphereMesh->friendlyName = "Sphere";
// 
// //        sphereMesh.physProps.velocity.y = 0.0f;
//         sPhsyicsProperties* pSpherePhysProps = new sPhsyicsProperties();        // HEAP
// 
//         pSpherePhysProps->velocity.y = getRandomFloat(0.0f, MAX_VELOCITY);
//         pSpherePhysProps->velocity.x = getRandomFloat(-MAX_VELOCITY, MAX_VELOCITY);
//         pSpherePhysProps->velocity.z = getRandomFloat(-MAX_VELOCITY, MAX_VELOCITY);
// 
//         // Gravity on Earth is likely too fast to look good, so we'll make it smaller.
// //        sphereMesh.physProps.acceleration.y = -9.81f;
//         pSpherePhysProps->acceleration.y = (-9.81f / 5.0f);
// 
//         pSpherePhysProps->position.x = getRandomFloat(-MAX_SPHERE_LOCATION, MAX_SPHERE_LOCATION);
//         pSpherePhysProps->position.z = getRandomFloat(-MAX_SPHERE_LOCATION, MAX_SPHERE_LOCATION);
//         pSpherePhysProps->position.y = getRandomFloat(10.0f, MAX_SPHERE_LOCATION + 20.0f);
// 
//         pSphereMesh->pPhysProps = pSpherePhysProps;
//         // Copy the physics position to the drawing position
//         pSphereMesh->drawPosition = pSpherePhysProps->position;
// 
//         ::g_vec_pMeshesToDraw.push_back(pSphereMesh);
// 
// 
//         //cMesh shpereMeshShadow_HACK;
//         //shpereMeshShadow_HACK.meshName = "Sphere_1_unit_Radius.ply";
//         //shpereMeshShadow_HACK.bIsWireframe = false;
//         //shpereMeshShadow_HACK.bDoNotLight = true;
// //        shpereMeshShadow_HACK.
// //        shpereMeshShadow_HACK.friendlyName = "Sphere";
// 
// 
//     }//for ( unsigned int count...

//     cMesh* pDebugSphere = new cMesh();
//     pDebugSphere->meshName = "Sphere_1_unit_Radius.ply";
//     pDebugSphere->bIsWireframe = true;
//     pDebugSphere->bDoNotLight = true;
//     pDebugSphere->scale = 25.0f;
//     pDebugSphere->pPhysProps = NULL;
//     pDebugSphere->bIsVisible = false;
//     pDebugSphere->friendlyName = "DEBUG_SPHERE";
// 
//     ::g_vec_pMeshesToDraw.push_back(pDebugSphere);



    return true;
}
