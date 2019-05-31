/*
 * TriangleBatch.cpp
 *
 *  Created on: Aug 6, 2018
 *      Author: Samuel Tan
 */

#include "graphics/TriangleBatch.h"
#include "Renderer.h"

namespace graphics
{

  TriangleBatch::TriangleBatch(unsigned int maxVertices,
      ShaderProgram& sp)
  : maxVertices(maxVertices),
    shaderProgram(sp), vertexArray(),
    vertexBuffer(nullptr, maxVertices * FLOATS_PER_VERTEX * sizeof(float), GL_STREAM_DRAW),
    indexBuffer(nullptr, maxVertices * 3, GL_STREAM_DRAW),
    iIndex(0), vIndex(0), vertexCount(0),
    iLength(maxVertices * 3), vLength(maxVertices * FLOATS_PER_VERTEX),
    iData(new GLuint[iLength]), vData(new float[vLength])
  {
    vertexArray.addAttribute("color", GL_FLOAT, 4);
    vertexArray.addAttribute("position", GL_FLOAT, 3);
    vertexArray.applyAttributesWithBuffer(vertexBuffer, shaderProgram);

    //indexBuffer.forceBind(); // ???????? idk how but it's needed
	// Nvm it's not needed now, i changed Renderer::draw() to use ib.forceBind()
	// hmm also works if i change IndexBuffer::updateData to use forceBind()
	// idk why it doesn't work to just use bind()
  }

  TriangleBatch::~TriangleBatch()
  {
    delete[] iData;
    delete[] vData;
  }

  bool TriangleBatch::submit(const graphics::Triangle& tri)
  {
    if (vertexCount + 3 > maxVertices)
      return false;

    for (const graphics::ColoredVertex& vertex : tri)
    {
      vData[vIndex++] = vertex.color.r;
      vData[vIndex++] = vertex.color.g;
      vData[vIndex++] = vertex.color.b;
      vData[vIndex++] = vertex.color.a;

      vData[vIndex++] = vertex.position.x;
      vData[vIndex++] = vertex.position.y;
      vData[vIndex++] = vertex.position.z;
    }

    iData[iIndex++] = vertexCount + 0;
    iData[iIndex++] = vertexCount + 1;
    iData[iIndex++] = vertexCount + 2;

    vertexCount += 3;

    return true;
  }

  bool TriangleBatch::submit(const graphics::Quad& quad)
  {
    if (vertexCount + 4 > maxVertices)
      return false;

    // do vertex buffer, four vertices
    for (const graphics::ColoredVertex& vertex : quad)
    {
      vData[vIndex++] = vertex.color.r;
      vData[vIndex++] = vertex.color.g;
      vData[vIndex++] = vertex.color.b;
      vData[vIndex++] = vertex.color.a;

      vData[vIndex++] = vertex.position.x;
      vData[vIndex++] = vertex.position.y;
      vData[vIndex++] = vertex.position.z;
    }

    // do index buffer, two triangles
    iData[iIndex++] = vertexCount + 0;    //    1---------0
    iData[iIndex++] = vertexCount + 1;    //    |         |
    iData[iIndex++] = vertexCount + 3;    //    |         |
    iData[iIndex++] = vertexCount + 1;    //    |         |
    iData[iIndex++] = vertexCount + 3;    //    |         |
    iData[iIndex++] = vertexCount + 2;    //    2---------3

    vertexCount += 4;

    return true;
  }

  bool TriangleBatch::submit(const graphics::CenteredPoly& cp)
  {

    if (vertexCount + cp.size() > maxVertices)
      return false;

    assert(cp.size() >= 3);

    // do vertex buffer
    for (const graphics::ColoredVertex& vertex : cp)
    {
      vData[vIndex++] = vertex.color.r;
      vData[vIndex++] = vertex.color.g;
      vData[vIndex++] = vertex.color.b;
      vData[vIndex++] = vertex.color.a;

      vData[vIndex++] = vertex.position.x;
      vData[vIndex++] = vertex.position.y;
      vData[vIndex++] = vertex.position.z;
    }

    // do index buffer - this is the tricky part
    for (unsigned int cpVertex = 1; cpVertex < cp.size() - 1; ++cpVertex)
    {
      // triangle
      iData[iIndex++] = vertexCount; // central vertex
      iData[iIndex++] = vertexCount + cpVertex;
      iData[iIndex++] = vertexCount + cpVertex + 1;
    }

    // last triangle in index buffer
    iData[iIndex++] = vertexCount;                 // central vertex
    iData[iIndex++] = vertexCount + cp.size() - 1; // last vertex
    iData[iIndex++] = vertexCount + 1;             // first non-center vertex

    // prepare for next cp
    vertexCount += cp.size();

    return true;
  }

  bool TriangleBatch::submit(const std::vector<graphics::ColoredVertex>& vertices, const std::vector<int>& indices)
  {
	  if (vertexCount + vertices.size() > maxVertices)
		  return false;

	  assert(vertices.size() >= 3);
	  assert(indices.size() % 3 == 0);

	  for (const graphics::ColoredVertex& vertex : vertices)
	  {
		  vData[vIndex++] = vertex.color.r;
		  vData[vIndex++] = vertex.color.g;
		  vData[vIndex++] = vertex.color.b;
		  vData[vIndex++] = vertex.color.a;

		  vData[vIndex++] = vertex.position.x;
		  vData[vIndex++] = vertex.position.y;
		  vData[vIndex++] = vertex.position.z;
	  }

	  for (const int& index : indices)
		  iData[iIndex++] = vertexCount + index;

	  vertexCount += vertices.size();

	  return true;
  }

  void TriangleBatch::renderAndClearAll()
  {
	  //// fill the rest of the arrays with 0
	  //for (; vIndex < vLength; ++vIndex)
	  //  vData[vIndex] = 0;
	  //for (; iIndex < iLength; ++iIndex)
	  //  iData[iIndex] = 0;


	  //// load array data into GL buffer objects
	  //indexBuffer.updateData(iData, iLength);
	  //vertexBuffer.updateData(vData, vLength * sizeof(float));

	  //Renderer::draw(GL_TRIANGLES, vertexArray, indexBuffer, shaderProgram);

	  //// next render session, whatever's in our member arrays will be overwritten
	  //iIndex = 0;
	  //vIndex = 0;
	  //vertexCount = 0;

	  // first time through, this will initialize iData and vData
	  static unsigned int iPopulatedPrev = iLength, vPopulatedPrev = vLength;
	  static bool firstRun = true;

	  // overwrite any remaining data in arrays from previous render
	  for (unsigned int j = vIndex; j < vPopulatedPrev; ++j)
		  vData[j] = 0.0f;
	  for (unsigned int j = iIndex; j < iPopulatedPrev; ++j)
		  iData[j] = 0;

	  iPopulatedPrev = iIndex;
	  vPopulatedPrev = vIndex;

	  // load array data into GL buffer objects
	  if (firstRun)
	  {
		  // initialize GL buffers to full size
		  indexBuffer.updateData(iData, iLength);
		  vertexBuffer.updateData(vData, vLength * sizeof(float));
	  }
	  else
	  {
		  // just update whatever portion is needed
		  indexBuffer.updateData(iData, iIndex);
		  vertexBuffer.updateData(vData, vIndex * sizeof(float));
	  }

	  Renderer::draw(GL_TRIANGLES, vertexArray, indexBuffer, shaderProgram);

	  // next render session, whatever's in iData and vData will be overwritten
	  iIndex = 0;
	  vIndex = 0;
	  vertexCount = 0;
  }

} /* namespace graphics */

