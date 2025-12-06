#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "maritime_system.h"
#include "menu.hpp"

using namespace sf;
using namespace std;

int main() // Entry point
{
    cout << "\n";
    cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    cout << "║        OCEANROUTE NAV - Maritime Navigation Optimizer           ║\n";
    cout << "║        Intelligent Route Planning & Visualization System        ║\n";
    cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";

    
    Font font;
    if (!font.loadFromFile("font/Neon 2 News.ttf"))
    {
        cerr << "Warning: Could not load font 'Neon 2 News.ttf'. Using default.\n";
    }

    RenderWindow window(VideoMode(1920, 1080), "OceanRoute Nav", Style::Fullscreen);
    window.setFramerateLimit(60);

    {
        Menu menu(window);
        Menu::Result choice = menu.run();

        if (choice == Menu::Result::Exit)
            return 0;
        else if (choice == Menu::Result::StartNavigation)
        {
        }
        else
        {
            return 0;
        }
    }

    MaritimeSystem system;
    system.runTests();

    Clock clock;

    while (window.isOpen()) // Main loop
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Escape)
                    window.close();

                system.handleKey(event.key.code);
            }

            if (event.type == Event::MouseButtonPressed)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                bool isRightClick = (event.mouseButton.button == Mouse::Right);
                system.handleClick(mousePos, isRightClick);
            }

            if (event.type == Event::MouseMoved)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                system.handleHover(mousePos);
            }

            if (event.type == Event::TextEntered && system.isInputActive())
            {
                system.handleText(event.text.unicode);
            }
        }

        float deltaTime = clock.restart().asSeconds();
        system.update(deltaTime);

        window.clear(Color(10, 20, 40));
        system.draw(window);
        window.display();
    }

    return 0;
}