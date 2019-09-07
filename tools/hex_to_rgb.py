colors = [
	'AE1616',
	'D67A0E',
	'E8D322',
	'66B512',
	'1EBF93',
	'359CF3',
]

for c in colors:
	print("sf::Color", tuple(int(c[i:i+2], 16) for i in (0, 2, 4)))