#include "cVAOManager.h"

#include "../OpenGLCommon.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

#include <sstream>
#include <fstream>


sModelDrawInfo::sModelDrawInfo()
{
	this->VAO_ID = 0;

	this->VertexBufferID = 0;
	this->VertexBuffer_Start_Index = 0;
	this->numberOfVertices = 0;

	this->IndexBufferID = 0;
	this->IndexBuffer_Start_Index = 0;
	this->numberOfIndices = 0;
	this->numberOfTriangles = 0;

	// The "local" (i.e. "CPU side" temporary array)
	this->pVertices = 0;	// or NULL
	this->pIndices = 0;		// or NULL

	// You could store the max and min values of the 
	//  vertices here (determined when you load them):
	glm::vec3 maxValues;
	glm::vec3 minValues;

//	scale = 5.0/maxExtent;		-> 5x5x5
	float maxExtent;

	return;
}


void cVAOManager::setBasePath(std::string basePathWithoutSlash)
{
    this->m_basePathWithoutSlash = basePathWithoutSlash;
    return;
}



bool cVAOManager::LoadModelIntoVAO(
		std::string fileName, 
		sModelDrawInfo &drawInfo,
	    unsigned int shaderProgramID)

{
	// Load the model from file
	// (We do this here, since if we can't load it, there's 
	//	no point in doing anything else, right?)

	drawInfo.meshName = fileName;

    std::string fileAndPath = this->m_basePathWithoutSlash + "/" + fileName;

    if ( ! this->m_LoadTheFile_Ply_XYZ_N_RGBA(fileAndPath, drawInfo) )
    {
        return false;
    };


	// ***********************************************************
	// TODO: Load the model from file

	// ***********************************************************
	// 
	// 
	// Model is loaded and the vertices and indices are in the drawInfo struct
	// 

	// Create a VAO (Vertex Array Object), which will 
	//	keep track of all the 'state' needed to draw 
	//	from this buffer...

	// Ask OpenGL for a new buffer ID...
	glGenVertexArrays( 1, &(drawInfo.VAO_ID) );
	// "Bind" this buffer:
	// - aka "make this the 'current' VAO buffer
	glBindVertexArray(drawInfo.VAO_ID);

	// Now ANY state that is related to vertex or index buffer
	//	and vertex attribute layout, is stored in the 'state' 
	//	of the VAO... 


	// NOTE: OpenGL error checks have been omitted for brevity
//	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &(drawInfo.VertexBufferID) );

//	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, drawInfo.VertexBufferID);
	// sVert vertices[3]
	glBufferData( GL_ARRAY_BUFFER, 
				  sizeof(sVertex) * drawInfo.numberOfVertices,	// ::g_NumberOfVertsToDraw,	// sizeof(vertices), 
				  (GLvoid*) drawInfo.pVertices,							// pVertices,			//vertices, 
				  GL_STATIC_DRAW );


	// Copy the index buffer into the video card, too
	// Create an index buffer.
	glGenBuffers( 1, &(drawInfo.IndexBufferID) );

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawInfo.IndexBufferID);

	glBufferData( GL_ELEMENT_ARRAY_BUFFER,			// Type: Index element array
	              sizeof( unsigned int ) * drawInfo.numberOfIndices, 
	              (GLvoid*) drawInfo.pIndices,
                  GL_STATIC_DRAW );

	// Set the vertex attributes.

	GLint vpos_location = glGetAttribLocation(shaderProgramID, "vPos");	// program
	GLint vcol_location = glGetAttribLocation(shaderProgramID, "vCol");	// program;
	GLint vNormal_location = glGetAttribLocation(shaderProgramID, "vNormal");	// program;

	// Set the vertex attributes for this shader
	glEnableVertexAttribArray(vpos_location);	    // vPos
	glVertexAttribPointer( vpos_location, 4,		// vPos
						   GL_FLOAT, GL_FALSE,
						   sizeof(sVertex), 
						   ( void* ) offsetof(sVertex, x));

	glEnableVertexAttribArray(vcol_location);	    // vCol
	glVertexAttribPointer( vcol_location, 4,		// vCol
						   GL_FLOAT, GL_FALSE,
                          sizeof(sVertex),
						   ( void* ) offsetof(sVertex, r));

	glEnableVertexAttribArray(vNormal_location);	// vNormal
	glVertexAttribPointer(vNormal_location, 4,		// vNormal
						   GL_FLOAT, GL_FALSE,
                           sizeof(sVertex),
						   ( void* ) offsetof(sVertex, nx));

	// Now that all the parts are set up, set the VAO to zero
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(vpos_location);
	glDisableVertexAttribArray(vcol_location);
	glDisableVertexAttribArray(vNormal_location);


	// Store the draw information into the map
	this->m_map_ModelName_to_VAOID[ drawInfo.meshName ] = drawInfo;


	return true;
}


// We don't want to return an int, likely
bool cVAOManager::FindDrawInfoByModelName(
		std::string filename,
		sModelDrawInfo &drawInfo) 
{
	std::map< std::string, sModelDrawInfo>::iterator itDrawInfo = this->m_map_ModelName_to_VAOID.find( filename );

	// Find it? 
	if ( itDrawInfo == this->m_map_ModelName_to_VAOID.end() )
	{
		// Nope
		return false;
	}

	// Else we found the thing to draw
	// ...so 'return' that information
	drawInfo = itDrawInfo->second;
	return true;
}



bool cVAOManager::m_LoadTheFile_Ply_XYZ_N_RGBA(std::string theFileName, sModelDrawInfo& drawInfo)
{
//    property float x
//    property float y
//    property float z

    //sVertexPlyFile p;       p.x = 0.0f;     p.y = 1.0f; p.z = 2.0f;
    //std::cout << p.x;

    //sVertexPlyFile q;
    //std::cout << "Type in the x: ";
    //std::cin >> q.x;

    // Input Filestream 
//    std::ifstream theBunnyFile("bathtub.ply");
    std::ifstream theBunnyFile( theFileName.c_str() );
    if (!theBunnyFile.is_open())
    {
        // didn't open the file.
        return false;
    }

    std::string temp;
    while (theBunnyFile >> temp)
    {
        if (temp == "vertex")
        {
            break;
        }
    };
    //element vertex 8171
//    theBunnyFile >> g_numberOfVertices;
    theBunnyFile >> drawInfo.numberOfVertices;


    while (theBunnyFile >> temp)
    {
        if (temp == "face")
        {
            break;
        }
    };
    //element vertex 8171
//    theBunnyFile >> g_numberOfTriangles;
    theBunnyFile >> drawInfo.numberOfTriangles;

    drawInfo.numberOfIndices = drawInfo.numberOfTriangles * 3;

    while (theBunnyFile >> temp)
    {
        if (temp == "end_header")
        {
            break;
        }
    };


    // Allocate enough space to hold the vertices
//    sVertex vertices[8171];                 // Stack

//    sVertex x;      // STACK based variable (on the stack)
//    sVertex* px;    // Pointer variable.

//    int y = 5;
//
//    int* py = new int();
//    *py = 5;



    // This most closely matches the ply file for the bunny
    struct sVertexPlyFile
    {
        float x;
        float y;
        float z;
        float nx, ny, nz;
        float r, g, b, a;
    };

    struct sTrianglePlyFile
    {
    //    sVertexPlyFile verts[3];
    //    unsigned int vertIndexes[3];
        // Vertices of the triangles
        unsigned int v0, v1, v2;
    };

    // Dynamically allocate memory on the heap;
//    sVertexPlyFile* pTheVerticesFile = new sVertexPlyFile[g_numberOfVertices];
    sVertexPlyFile* pTheVerticesFile = new sVertexPlyFile[drawInfo.numberOfVertices];

    // -0.036872 0.127727 0.00440925 
//    for (unsigned int index = 0; index != g_numberOfVertices; index++)
    for (unsigned int index = 0; index != drawInfo.numberOfVertices; index++)
    {
        sVertexPlyFile tempVertex;
        theBunnyFile >> tempVertex.x;                //std::cin >> a.x;
        theBunnyFile >> tempVertex.y;                //std::cin >> a.y;
        theBunnyFile >> tempVertex.z;                //std::cin >> a.z;

        theBunnyFile >> tempVertex.nx;
        theBunnyFile >> tempVertex.ny;
        theBunnyFile >> tempVertex.nz;

        tempVertex.r = 255.0f;
        tempVertex.g = 0.0f;
        tempVertex.b = 0.0f;
        tempVertex.a = 255.0f;


        pTheVerticesFile[index] = tempVertex;
    }


//    sTrianglePlyFile* pTheTriangles = new sTrianglePlyFile[g_numberOfTriangles];
    sTrianglePlyFile* pTheTriangles = new sTrianglePlyFile[drawInfo.numberOfTriangles];

    // 3 3495 3549 3548 
    for (unsigned int index = 0; index != drawInfo.numberOfTriangles; index++)
    {
        sTrianglePlyFile tempTriangle;

        unsigned int discard;
        theBunnyFile >> discard;            // 3
        theBunnyFile >> tempTriangle.v0;                //std::cin >> a.x;
        theBunnyFile >> tempTriangle.v1;                //std::cin >> a.y;
        theBunnyFile >> tempTriangle.v2;                //std::cin >> a.z;

        pTheTriangles[index] = tempTriangle;
    }

    // Before, we manually looked up the vertex indices and added them to the vertex buffer
    //g_NumberOfVerticesToDraw = g_numberOfTriangles * 3;
    //pVertices = new sVertex[g_NumberOfVerticesToDraw];

    //unsigned int vertIndex = 0;
    //for (unsigned int triIndex = 0; triIndex != g_numberOfTriangles; triIndex++)
    //{
    //    // 3 1582 1581 2063 
    //    pVertices[vertIndex + 0].x = pTheVerticesFile[pTheTriangles[triIndex].v0].x;
    //    pVertices[vertIndex + 0].y = pTheVerticesFile[pTheTriangles[triIndex].v0].y;
    //    pVertices[vertIndex + 0].z = pTheVerticesFile[pTheTriangles[triIndex].v0].z;

    //    pVertices[vertIndex + 0].r = pTheVerticesFile[pTheTriangles[triIndex].v0].r;
    //    pVertices[vertIndex + 0].g = pTheVerticesFile[pTheTriangles[triIndex].v0].g;
    //    pVertices[vertIndex + 0].b = pTheVerticesFile[pTheTriangles[triIndex].v0].b;

    //    pVertices[vertIndex + 1].x = pTheVerticesFile[pTheTriangles[triIndex].v1].x;
    //    pVertices[vertIndex + 1].y = pTheVerticesFile[pTheTriangles[triIndex].v1].y;
    //    pVertices[vertIndex + 1].z = pTheVerticesFile[pTheTriangles[triIndex].v1].z;

    //    pVertices[vertIndex + 1].r = pTheVerticesFile[pTheTriangles[triIndex].v1].r;
    //    pVertices[vertIndex + 1].g = pTheVerticesFile[pTheTriangles[triIndex].v1].g;
    //    pVertices[vertIndex + 1].b = pTheVerticesFile[pTheTriangles[triIndex].v1].b;

    //    pVertices[vertIndex + 2].x = pTheVerticesFile[pTheTriangles[triIndex].v2].x;
    //    pVertices[vertIndex + 2].y = pTheVerticesFile[pTheTriangles[triIndex].v2].y;
    //    pVertices[vertIndex + 2].z = pTheVerticesFile[pTheTriangles[triIndex].v2].z;

    //    pVertices[vertIndex + 2].r = pTheVerticesFile[pTheTriangles[triIndex].v2].r;
    //    pVertices[vertIndex + 2].g = pTheVerticesFile[pTheTriangles[triIndex].v2].g;
    //    pVertices[vertIndex + 2].b = pTheVerticesFile[pTheTriangles[triIndex].v2].b;

    //    vertIndex += 3;
    //}

    // ... now we just copy the vertices from the file as is (unchanged)
    drawInfo.pVertices = new sVertex[drawInfo.numberOfVertices];
    for (unsigned int vertIndex = 0; vertIndex != drawInfo.numberOfVertices; vertIndex++)
    {
        // 3 1582 1581 2063 
        drawInfo.pVertices[vertIndex].x = pTheVerticesFile[vertIndex].x;
        drawInfo.pVertices[vertIndex].y = pTheVerticesFile[vertIndex].y;
        drawInfo.pVertices[vertIndex].z = pTheVerticesFile[vertIndex].z;
        drawInfo.pVertices[vertIndex].w = 1.0f;


        drawInfo.pVertices[vertIndex].nx = pTheVerticesFile[vertIndex].nx;
        drawInfo.pVertices[vertIndex].ny = pTheVerticesFile[vertIndex].ny;
        drawInfo.pVertices[vertIndex].nz = pTheVerticesFile[vertIndex].nz;
        drawInfo.pVertices[vertIndex].nw = 1.0f;

        drawInfo.pVertices[vertIndex].r = pTheVerticesFile[vertIndex].r;
        drawInfo.pVertices[vertIndex].g = pTheVerticesFile[vertIndex].g;
        drawInfo.pVertices[vertIndex].b = pTheVerticesFile[vertIndex].b;
        drawInfo.pVertices[vertIndex].a = pTheVerticesFile[vertIndex].a;
    }

    // Allocate an array for all the indices (which is 3x the number of triangles)
    // Element array is an 1D array of integers
    drawInfo.pIndices = new unsigned int[drawInfo.numberOfIndices];

    unsigned int elementIndex = 0;
    for (unsigned int triIndex = 0; triIndex != drawInfo.numberOfTriangles; triIndex++)
    {
        // 3 1582 1581 2063 
        drawInfo.pIndices[elementIndex + 0] = pTheTriangles[triIndex].v0;
        drawInfo.pIndices[elementIndex + 1] = pTheTriangles[triIndex].v1;
        drawInfo.pIndices[elementIndex + 2] = pTheTriangles[triIndex].v2;

        elementIndex += 3;      // Next "triangle"
    }


    return true;
}

