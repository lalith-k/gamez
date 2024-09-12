#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

#include "olcConsoleGameEngine.h"

class game : public olcConsoleGameEngine
{
public :
	game()
	{
		m_sAppName = L"Asteroids";
	}

	void wrapCoordinates(float ix, float iy, float& ox, float& oy)
	{
		ox = ix;
		oy = iy;
		if (ix < 0.0f) ox = ix + (float)ScreenWidth();
		if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();
		if (iy < 0.0f) oy = iy + (float)ScreenHeight();
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
	}

	virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		wrapCoordinates(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}

	bool isPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
	}

	void resetGame()
	{
		vecAsteroids.clear();
		vecBullets.clear();

		vecAsteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16, 0.0f });
		vecAsteroids.push_back({ 200.0f, 20.0f, 5.0f, -6.0f, (int)16, 0.0f });
		vecAsteroids.push_back({ 100.0f, 20.0f, -5.0f, -6.0f, (int)16, 0.0f });

		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		bDead = false;
		nScore = 0;
	}

private:
	struct spaceObject
	{
		float x;
		float y;
		float dx;
		float dy;
		int nSize;
		float angle;
	};

	vector<spaceObject> vecAsteroids;
	vector<spaceObject> vecBullets;
	spaceObject player;
	bool bDead = false;
	int nScore = 0;

	vector<pair<float, float>> vecModelShip;
	vector<pair<float, float>> vecModelAsteroid;

protected:
	virtual bool OnUserCreate() 
	{
		vecModelShip =
		{
			{0.0f, -5.0f},
			{-2.5f,+2.5f},
			{+2.5f,+2.5f}
		};

		int verts = 20;
		for (int i = 0; i < verts; i++)
		{
			float radius = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;;
			float a = ((float)i / float(verts)) * 6.28318f;
			vecModelAsteroid.push_back(make_pair(radius * sinf(a), radius * cosf(a)));
		}

		resetGame();

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (bDead)
			resetGame();

		//Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		//steering the spaceship
		if (m_keys[VK_LEFT].bHeld)
			player.angle -= 5.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)
			player.angle += 5.0f * fElapsedTime;

		//Thrust
		if (m_keys[VK_UP].bHeld)
		{
			//accelaration changing velocity wrt time
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}


		//velocity changing position wrt time
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		//wrapping spaceship withing gamespace
		wrapCoordinates(player.x, player.y, player.x, player.y);

		// Check ship collision with asteroids
		for (auto& a : vecAsteroids)
		{
			if (isPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
			{
				bDead = true;
			}
		}

		//for bullets
		if (m_keys[VK_SPACE].bReleased)
			vecBullets.push_back({ player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 0, 0 });

		//Updating and drawing asteroids
		for (auto& a : vecAsteroids)
		{
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime;
			wrapCoordinates(a.x, a.y, a.x, a.y);

			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, a.nSize, FG_BLUE);
		}

		//temp vector to store new Asteroids cause if we update old vector eveything will crash
		vector<spaceObject> newAsteroids;

		//Updating and drawing bullets
		for (auto& b : vecBullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			wrapCoordinates(b.x, b.y, b.x, b.y);
			Draw(b.x, b.y);

			for (auto& a : vecAsteroids)
			{
				if (isPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					// Asteroid hit by bullet
					b.x = -100; // Remove bullet

					if (a.nSize > 4)
					{
						// Creating 2 child asteroids
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						newAsteroids.push_back({ a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), (int)a.nSize >> 1, 0.0f });
						newAsteroids.push_back({ a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), (int)a.nSize >> 1, 0.0f });
					}

					a.x = -100;
					nScore += 100;
				}
			}
		}

		// Append new astroids to existing vector
		for (auto a : newAsteroids)
		{
			vecAsteroids.push_back(a);
		}

		//Remove bullets which left screen
		if (vecBullets.size() > 0)
		{
			auto i = remove_if(vecBullets.begin(), vecBullets.end(), 
				[&](spaceObject o) {return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() - 1 || o.y >= ScreenHeight() - 1);  });

			if (i != vecBullets.end())
				vecBullets.erase(i);
		}

		// Remove broken asteroids
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), [&](spaceObject o) {return (o.x < 0); });
			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		if (vecAsteroids.empty())
		{
			nScore += 1000;
			// Add new asteroids 90 degrees to the left and right of player for next level

			vecAsteroids.push_back({
				30.0f * sinf(player.angle - 3.14159f / 2.0f),
				30.0f * cosf(player.angle - 3.14159f / 2.0f),
				10.0f * sinf(player.angle),
				10.0f * cosf(player.angle),
				(int)16, 0.0f });

			vecAsteroids.push_back({
				30.0f * sinf(player.angle + 3.14159f / 2.0f),
				30.0f * cosf(player.angle + 3.14159f / 2.0f),
				10.0f * sinf(player.angle),
				10.0f * cosf(player.angle),
				(int)16, 0.0f });
		}
		 
		//Draw spaceship
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

		//Draw score
		DrawString(2, 2, L"SCORE: " + to_wstring(nScore));

		return true;
	}


};

int main()
{
	game g;
	g.ConstructConsole(160,60,8,12);
	g.Start();
	return 0;
}

