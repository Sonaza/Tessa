#pragma once

#include <vector>

TS_PACKAGE2(game, gameplay)

class NonogramPuzzle
{
public:
	NonogramPuzzle(SizeType x, SizeType y);
	~NonogramPuzzle();

	sf::Vector2u getSize() const { return size; }

	enum SquareState {
		Unchecked,
		Checked,
		Marked,
	};
	struct Square
	{
		SquareState current = Unchecked;
		SquareState previous = Unchecked;
		sf::Time changedTime;
	};

	void setSquare(SizeType x, SizeType y, SquareState state);
	SquareState getSquare(SizeType x, SizeType y);

	sf::VertexArray getVertexArray(float squareSize = 30);

protected:
	sf::Vector2u size;
	std::vector<Square> grid;

};

TS_END_PACKAGE2()
