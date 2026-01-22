#pragma once
#include <iostream>
class Window
{
public:
	Window();
	~Window();
	void GetTiledSizeAndOrigin(int* usize, int* vsize, int* lowerLeftU, int* lowerLeftV);
	void NormalizedDisplayToDisplay(double& u, double& v);
	int* GetSize();
private:
	int m_iWidth;
	int m_iHeight;

	int Size[2];
	int TileSize[2];
	int TileScale[2];
};

