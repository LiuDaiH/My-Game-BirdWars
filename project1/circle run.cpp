#include <SFML/Graphics.hpp>
#include <windows.h>

int main()
{
	int x = 200, y = 40;
	int xv = 5, yv = 9;
	int ww = 800, hh = 400;
	
    sf::RenderWindow window(sf::VideoMode(ww, hh), "SFML works!");
    sf::CircleShape shape(20);
    shape.setFillColor(sf::Color(255, 128, 0));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

		{
			x += xv;
			y += yv;
			
			if (x <= 0 || x >= ww-10) xv = -xv; 
			if (y <= 0 || y >= hh-10) yv = -yv; 
			shape.setPosition(x, y);  
		}
        window.clear();
        window.draw(shape);
        window.display();
        
        Sleep(1);
    }

    return 0;
}


