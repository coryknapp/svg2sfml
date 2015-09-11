#include "svg2sfml.hpp"

#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <set>
#include <exception>

#include <math.h>

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

#include <SFML/Graphics.hpp>

#include "color_table.hpp"
#include <sfml-utilities/sfml-utilities.hpp>
namespace svg2sfml{

static const std::string default_fill_color = "#FFFFFF";
static const std::string default_stroke_color = "#0000FF";

return_t readSVG( const std::string &path ){
    Loader l;
    return l.read( path );
}
    
sf::Color colorFromSVGColor( const std::string &name ){


	if( name[0] == '#' ){
		int r = stoi( name.substr(1,2), nullptr, 16 );
		int g = stoi( name.substr(3,2), nullptr, 16 );
		int b = stoi( name.substr(5,2), nullptr, 16 );
		return sf::Color( r, g, b, 255 );
	}
	auto iter = color_names.find( name );
	if( iter != color_names.end() ){
		std::cout << name << std::endl;
		return colorFromSVGColor( iter->second );
	}
	std::cout << "Warning: no color found.  defaults to white." << std::endl;
	return sf::Color(255,255,255,255);//TODO throw something
}


void Loader::applyColorAsShape( sf::Shape * shape ){
//color/stroke attributes shared by all shapes
	shape->setFillColor( colorFromSVGColor(
			m_tagStack.back()->second.get<std::string>(
				"<xmlattr>.fill" , default_fill_color) ) );
	shape->setOutlineColor(
		colorFromSVGColor(
			m_tagStack.back()->second.get<std::string>(
				"<xmlattr>.stroke", default_stroke_color) ) );
	shape->setOutlineThickness(
		m_tagStack.back()->second.get<float>("<xmlattr>.stroke-width", 0.0f) );
}

void Loader::applyColorAsLine( sf::Shape * shape ){
//color/stroke attributes shared by all shapes
	shape->setFillColor( colorFromSVGColor(
			m_tagStack.back()->second.get<std::string>("<xmlattr>.stroke" , default_fill_color) ) );
	shape->setOutlineThickness( 0.0f );
}

std::unique_ptr< sf::Drawable >
	Loader::readAsRect(){
	sf::RectangleShape *newRect = new sf::RectangleShape;
		newRect->setPosition(
			m_tagStack.back()->second.get<float>("<xmlattr>.x"),
			m_tagStack.back()->second.get<float>("<xmlattr>.y") );
		newRect->setSize( sf::Vector2f(
			m_tagStack.back()->second.get<float>("<xmlattr>.width"),
			m_tagStack.back()->second.get<float>("<xmlattr>.height") ) );
		//TODO support rounded rectangles
		//returnShape->setRoundedRectangles(
		//	m_tagStack.back()->second.get<float>("rx"),
		//	m_tagStack.back()->second.get<float>("ry") );
	applyColorAsShape( newRect );
	return std::unique_ptr< sf::Drawable >( newRect );
}

std::unique_ptr< sf::Drawable >
	Loader::readAsCircle(){
	sf::CircleShape * newCirc = new sf::CircleShape;
	newCirc->setPosition(
		m_tagStack.back()->second.get<float>("<xmlattr>.cx"),
		m_tagStack.back()->second.get<float>("<xmlattr>.cy") );
	newCirc->setOrigin(
		m_tagStack.back()->second.get<float>("<xmlattr>.r"),
		m_tagStack.back()->second.get<float>("<xmlattr>.r"));

	newCirc->setRadius(
		m_tagStack.back()->second.get<float>("<xmlattr>.r") );
	applyColorAsShape( newCirc );
	return std::unique_ptr< sf::Drawable >( newCirc );
}


std::unique_ptr< sf::Drawable >rectForLine(
		float x1,
		float y1,
		float x2,
		float y2,
		float width ){
	float length = sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
	sf::RectangleShape *newRect = new sf::RectangleShape;
	newRect->setPosition(x1, y1);
	newRect->setSize( sf::Vector2f( length+width, width ) );
	newRect->setOrigin( width/2.0f, width/2.0f );
	newRect->setRotation(
		180.0f/M_PI * sfu::angleOff0( sf::Vector2f( x2-x1, y2-y1 ) ) );
	return std::unique_ptr< sf::Drawable >( newRect );
}

std::unique_ptr< sf::Drawable > Loader::readAsLine(){
	float x1 = m_tagStack.back()->second.get<float>("<xmlattr>.x1");
	float y1 = m_tagStack.back()->second.get<float>("<xmlattr>.y1");
	float x2 = m_tagStack.back()->second.get<float>("<xmlattr>.x2");
	float y2 = m_tagStack.back()->second.get<float>("<xmlattr>.y2");
	float width = m_tagStack.back()->second.get<float>("<xmlattr>.stroke-width");
	auto retRect = rectForLine( x1, y1, x2, y2, width );
    applyColorAsLine( boost::polymorphic_downcast<sf::Shape*>(retRect.get()) );
	return retRect;
}

return_t Loader::readAsPolyline(){
    return_t returnPool;
    std::string points = m_tagStack.back()->second.get<std::string>("<xmlattr>.points");
    float width = m_tagStack.back()->second.get<float>("<xmlattr>.stroke-width");
    std::stringstream dStream( std::move(points) );
	float nX, nY, oX, oY;
    std::string pairString;
    
    //read first pair
    dStream >> pairString;
    size_t commaPos = pairString.find( "," );
    oX = boost::lexical_cast<float>( pairString.substr(0, commaPos) );
    oY = boost::lexical_cast<float>( pairString.substr(commaPos+1) );
    
    while( !dStream.eof() ){
        //the polyline path element is formed by pairs of numbers,
        //where each coordinate is separated by a comma, and each
        //pair is separated by a whitespace
		dStream >> pairString;
		size_t commaPos = pairString.find( "," );
        nX = boost::lexical_cast<float>( pairString.substr(0, commaPos) );
        nY = boost::lexical_cast<float>( pairString.substr(commaPos+1) );

        auto lineSegRect = rectForLine( oX, oY, nX, nY, width );
        applyColorAsLine( boost::polymorphic_downcast<sf::Shape*>(lineSegRect.get()) );
        returnPool.push_back( std::move( lineSegRect ) );
        
        oX = nX;
        oY = nY;
    }
    return returnPool;
}
    
return_t Loader::readAsPolygon(){
    return_t returnPool;
    std::string points = m_tagStack.back()->second.get<std::string>("<xmlattr>.points");
    std::stringstream dStream( std::move(points) );
    
    sf::ConvexShape * shape = new sf::ConvexShape;
    
    std::string pairString;
    float x, y;
    int pCount = 0;
    while( !dStream.eof() ){
        //the polyline path element is formed by pairs of numbers,
        //where each coordinate is separated by a comma, and each
        //pair is separated by a whitespace
        dStream >> pairString;
        size_t commaPos = pairString.find( "," );
        x = boost::lexical_cast<float>( pairString.substr(0, commaPos) );
        y = boost::lexical_cast<float>( pairString.substr(commaPos+1) );
        
        pCount++;
        shape->setPointCount( pCount );
        shape->setPoint( pCount, sf::Vector2f( x, y) );
    }
    applyColorAsShape( shape );
    returnPool.push_back(std::unique_ptr<sf::Drawable>( shape ));
    return returnPool;
}
    
return_t Loader::shapesFromSVGTag(){
    assert( !m_tagStack.empty() );
	return_t shapes;
	if( m_tagStack.back()->first == "rect" ){
		shapes.push_back( readAsRect() );
	} else if( m_tagStack.back()->first == "circle" ) {
		shapes.push_back( readAsCircle() );
	} else if( m_tagStack.back()->first == "ellipse" ) {
		//shapes.push_back( readAsEllipse() );
	} else if( m_tagStack.back()->first == "line" ) {
		shapes.push_back( readAsLine() );
	} else if( m_tagStack.back()->first == "polyline" ) {
		shapes = readAsPolyline();
	}

	return std::move( shapes );
}

return_t Loader::read( const std::string &path ){
	
	return_t returnVector;

    pt::ptree tree;
    pt::read_xml(path, tree);

    for(pt::ptree::value_type &shapeNode : tree.get_child("svg")) {
        // The data function is used to access the data stored in a node.
		try{
			//only needed to trigger the catch.  must be a better way
			//auto &attributes =  shapeNode.second.get_child("<xmlattr>");
            //TODO implement searching the stack to get all properties
            m_tagStack.push_back( &shapeNode );
			auto shapes = shapesFromSVGTag();
			for (auto &shape : shapes) {
				if( shape.get() != nullptr )
					returnVector.push_back( std::move( shape ) );
			}
            m_tagStack.pop_back();
		} catch( pt::ptree_bad_path e ){
			std::cout << "bad path = " << e.what()<< std::endl;
		}
	}


	return returnVector;
}

}//end of svg2sfml namespace
