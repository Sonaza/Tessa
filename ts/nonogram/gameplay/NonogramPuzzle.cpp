#include "Precompiled.h"
#include "ts/nonogram/gameplay/NonogramPuzzle.h"

#include "ts/tessa/system/Timer.h"

TS_PACKAGE2(game, gameplay)

NonogramPuzzle::NonogramPuzzle(SizeType x, SizeType y)
	: size(x, y)
{
	grid.resize(x * y);

	for (SizeType i = 0; i < grid.size(); ++i)
	{
		grid[i].current = (SquareState)(i % 3);
	}
}

NonogramPuzzle::~NonogramPuzzle()
{

}

NonogramPuzzle::SquareState NonogramPuzzle::getSquare(SizeType x, SizeType y)
{
	SizeType index = y * size.x + x;
	TS_ASSERT(index < grid.size());

	if (index < grid.size())
		return grid[index].current;

	return Unchecked;
}

void NonogramPuzzle::setSquare(SizeType x, SizeType y, SquareState state)
{
	SizeType index = y * size.x + x;
	TS_ASSERT(index < grid.size());

	if (index < grid.size())
	{
		grid[index].previous = grid[index].current;
		grid[index].current = state;
		grid[index].changedTime = system::Timer::getTime();
	}
}

sf::VertexArray NonogramPuzzle::getVertexArray(float squareSize)
{
	sf::VertexArray array(sf::Quads);

	for (SizeType y = 0; y < size.y; ++y)
	{
		for (SizeType x = 0; x < size.x; ++x)
		{
			SizeType index = y * size.x + x;

			switch (grid[index].current)
			{
				case Unchecked:
				{
					// Nada
				}
				break;

				case Checked:
				{
					sf::Vertex v[4];
					v[0].position = sf::Vector2f((x + 0) * squareSize, (y + 0) * squareSize);
					v[1].position = sf::Vector2f((x + 1) * squareSize, (y + 0) * squareSize);
					v[2].position = sf::Vector2f((x + 1) * squareSize, (y + 1) * squareSize);
					v[3].position = sf::Vector2f((x + 0) * squareSize, (y + 1) * squareSize);
					
					for (SizeType i = 0; i < 4; ++i)
					{
						v[i].color = sf::Color(255, 255, 255);
						array.append(v[i]);
					}
				}
				break;

				case Marked:
				{

					sf::Vertex v[4];
					v[0].position = sf::Vector2f((x + 0) * squareSize + squareSize * 0.3f, (y + 0) * squareSize + squareSize * 0.3f);
					v[1].position = sf::Vector2f((x + 1) * squareSize - squareSize * 0.3f, (y + 0) * squareSize + squareSize * 0.3f);
					v[2].position = sf::Vector2f((x + 1) * squareSize - squareSize * 0.3f, (y + 1) * squareSize - squareSize * 0.3f);
					v[3].position = sf::Vector2f((x + 0) * squareSize + squareSize * 0.3f, (y + 1) * squareSize - squareSize * 0.3f);

					for (SizeType i = 0; i < 4; ++i)
					{
						v[i].color = sf::Color(255, 50, 50);
						array.append(v[i]);
					}
				}
				break;
			}
		}
	}

	return array;
}

TS_END_PACKAGE2()
