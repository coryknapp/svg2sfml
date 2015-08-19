#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

#include "svg2sfml.hpp"

int main(int argc, char *argv[])
{
    // create the window
    sf::RenderWindow window(sf::VideoMode(800, 600), "svg2sfml test");

	auto svgs = svg2sfml::readSVG( "test.svg" );

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
		for (auto &e : svgs) {
			window.draw( *e.get() );
		}

        // end the current frame
        window.display();
    }

    return 0;
}
