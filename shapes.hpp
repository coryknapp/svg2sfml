//
//  shapes.hpp
//  svgExample
//
//  Created by Cory Knapp on 9/10/15.
//  Copyright (c) 2015 Cory Knapp. All rights reserved.
//

#ifndef svgExample_shapes_hpp
#define svgExample_shapes_hpp

#include <SFML/Graphics.hpp>

namespace svg2sfml {

//ripped from http://www.sfml-dev.org/tutorials/2.0/graphics-shape.php
class EllipseShape : public sf::Shape
{
public:
    
    explicit EllipseShape(const sf::Vector2f& radius = sf::Vector2f(0, 0)) :
        m_radius(radius){
        update();
    }
    
    void setRadius(const sf::Vector2f& radius){
        m_radius = radius;
        update();
    }
    
    const sf::Vector2f& getRadius() const{
        return m_radius;
    }
    
    virtual size_t getPointCount() const{
        return 30; // fixed, but could be an attribute of the class if needed
    }
    
    virtual sf::Vector2f getPoint(size_t index) const{
        static const float pi = 3.141592654f;
        
        float angle = index * 2 * pi / getPointCount() - pi / 2;
        float x = std::cos(angle) * m_radius.x;
        float y = std::sin(angle) * m_radius.y;
        
        return sf::Vector2f(m_radius.x + x, m_radius.y + y);
    }
    
private:
    
    sf::Vector2f m_radius;
};
}

#endif
