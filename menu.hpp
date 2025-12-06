#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;
class Menu // Menu class
{
public:
    enum class Result
    {
        None,
        StartNavigation,

        Exit,

    };

    RenderWindow &m_window;
    Font m_font;
    Texture m_background_texture;
    Sprite m_background;
    Text m_title;
    Music m_background_music;

    static const int MAX_ITEMS = 10;
    Text m_items[MAX_ITEMS];
    const char *m_labels[MAX_ITEMS]{};
    int m_item_count;

    int m_selected_index;
    bool m_is_active;

    Menu(RenderWindow &window)
        : m_window(window), m_item_count(0), m_selected_index(0), m_is_active(true)
    {

        m_labels[0] = "Start Navigation";

        m_labels[1] = "Exit";
        m_item_count = 2;

        if (!m_font.loadFromFile("./font/Neon 2 News.ttf"))
        {
        }

        if (!m_background_music.openFromFile("music/song.ogg"))
        {
        }
        else
        {
            m_background_music.setVolume(100.f);
            m_background_music.setLoop(true);
            m_background_music.play();
        }
        if (m_background_texture.loadFromFile("./texture/wmremove-transformed.jpeg"))
        {
            m_background.setTexture(m_background_texture);
            Vector2u tex = m_background_texture.getSize();
            Vector2u win = m_window.getSize();
            float scale = max(float(win.x) / tex.x, float(win.y) / tex.y);
            m_background.setScale(scale, scale);
        }

        m_title.setFont(m_font);
        m_title.setString("OceanNav");
        m_title.setCharacterSize(142);
        m_title.setFillColor(Color::Black);
        m_title.setStyle(Text::Bold);
        FloatRect tb = m_title.getLocalBounds();
        m_title.setOrigin(tb.width / 2, tb.height / 2);
        m_title.setPosition(m_window.getSize().x / 2.f, 125.f);

        for (int i = 0; i < m_item_count; ++i)
        {
            m_items[i].setFont(m_font);
            m_items[i].setString(m_labels[i]);
            m_items[i].setCharacterSize(96);
            m_items[i].setFillColor(Color::White);
            FloatRect ib = m_items[i].getLocalBounds();
            m_items[i].setOrigin(ib.width / 2, ib.height / 2);
            m_items[i].setPosition(m_window.getSize().x / 2.f, 300.f + i * 100.f);
        }
        m_items[0].setFillColor(Color::White);
        m_items[0].setStyle(Text::Bold);
    }

    void handle_event(const Event &event)
    {
        if (!m_is_active)
            return;

        if (event.type == Event::KeyPressed)
        {
            if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W)
                move_up();
            if (event.key.code == Keyboard::Down || event.key.code == Keyboard::S)
                move_down();
            if (event.key.code == Keyboard::Enter || event.key.code == Keyboard::Space)
                m_is_active = false;
            if (event.key.code == Keyboard::Escape)
                m_is_active = false;
        }

        if (event.type == Event::MouseMoved || event.type == Event::MouseButtonPressed)
        {
            Vector2f mouse = m_window.mapPixelToCoords(Mouse::getPosition(m_window));
            for (int i = 0; i < m_item_count; ++i)
            {
                if (m_items[i].getGlobalBounds().contains(mouse))
                {
                    if (i != m_selected_index)
                    {
                        m_items[m_selected_index].setFillColor(Color::White);
                        m_items[m_selected_index].setStyle(Text::Regular);
                        m_selected_index = i;
                        m_items[i].setFillColor(Color::Blue);
                        m_items[i].setStyle(Text::Bold);
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                    {
                        m_is_active = false;
                    }
                }
            }
        }
    }
    void update()
    {
    }

    void draw()
    {
        m_window.clear(Color::Blue);
        if (m_background_texture.getSize().x > 0)
            m_window.draw(m_background);
        m_window.draw(m_title);
        for (int i = 0; i < m_item_count; ++i)
            m_window.draw(m_items[i]);
    }

    Result run()
    {
        m_is_active = true;
        m_selected_index = 0;
        m_items[0].setFillColor(Color::Blue);
        m_items[0].setStyle(Text::Bold);

        while (m_window.isOpen() && m_is_active)
        {
            Event event;
            while (m_window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                    return Result::Exit;
                handle_event(event);
            }
            update();
            draw();
            m_window.display();
        }

        switch (m_selected_index)
        {
        case 0:

            return Result::StartNavigation;

        case 3:
            return Result::Exit;
            ;
        default:
            return Result::None;
        }
    }

    void move_up()
    {
        m_items[m_selected_index].setFillColor(Color::White);
        m_items[m_selected_index].setStyle(Text::Regular);
        m_selected_index = (m_selected_index == 0) ? m_item_count - 1 : m_selected_index - 1;
        m_items[m_selected_index].setFillColor(Color::Blue);
        m_items[m_selected_index].setStyle(Text::Bold);
    }

    void move_down()
    {
        m_items[m_selected_index].setFillColor(Color::White);
        m_items[m_selected_index].setStyle(Text::Regular);
        m_selected_index = (m_selected_index + 1) % m_item_count;
        m_items[m_selected_index].setFillColor(Color::Blue);
        m_items[m_selected_index].setStyle(Text::Bold);
    }
};