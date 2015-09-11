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
#include "shapes.hpp"
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
    boost::optional<std::string> fillColorStr;
    boost::optional<std::string> strokeColorStr;
    boost::optional<std::string> thicknessStr;
    for( auto &e : m_tagStack ){
        auto tempFill = e->second.get_optional<std::string>( "<xmlattr>.fill" );
        if( tempFill )
            fillColorStr = std::move( tempFill );
        auto tempStroke = e->second.get_optional<std::string>( "<xmlattr>.stroke" );
        if( tempStroke )
            strokeColorStr = std::move( tempStroke );
        auto tempWidth = e->second.get_optional<std::string>( "<xmlattr>.stroke-width" );
        if( tempWidth )
            thicknessStr = std::move( tempWidth );
    }
    
    std::cout << "DICK " << fillColorStr.value_or( "SHIT FUCKER" ) << "\n";
    
    shape->setFillColor( colorFromSVGColor( fillColorStr.value_or( default_fill_color) ) );
    shape->setOutlineColor( colorFromSVGColor( strokeColorStr.value_or( default_stroke_color ) ) );
    shape->setOutlineThickness( boost::lexical_cast<float>( thicknessStr.value_or( "0" ) ) );
}

void Loader::applyColorAsLine( sf::Shape * shape ){
//color/stroke attributes shared by all shapes
	shape->setFillColor( colorFromSVGColor(
			m_tagStack.back()->second.get<std::string>("<xmlattr>.stroke" , default_fill_color) ) );
	shape->setOutlineThickness( 0.0f );
}
    

void Loader::applyTransformable( sf::Shape * shape ){
    for( auto &e : m_tagStack ){
        auto transString = e->second.get_optional<std::string>( "<xmlattr>.transform" );
        if( transString ){
            std::stringstream tStream( transString.value() );
            std::string command;
            while( !tStream.eof() ){
                tStream >> command;
                //find what comes before the (, that's the command
                size_t openParan = command.find( "(" );
                std::string functionName = command.substr( 0, openParan );
                std::string argument = command.substr( openParan+1, command.find( ")" ) );
                if( functionName == "translate" ){
                    //this one has two coords
                    size_t commaPos = argument.find( "," );
                    float x = boost::lexical_cast<float>( argument.substr(0, commaPos) );
                    float y = boost::lexical_cast<float>( argument.substr(commaPos+1) );
                    shape->move( x, y );
                } else if( functionName == "scale"){
                    float s = boost::lexical_cast<float>( argument );
                    shape->scale( s, s );
                } else if( functionName == "rotate"){
                    float r = boost::lexical_cast<float>( argument );
                    shape->rotate( r );
                } else {
                    std::cout << "Don't know what to do with \"" << functionName << "\"\n";
                }
            }
        }
    }
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
    
std::unique_ptr< sf::Drawable >
    Loader::readAsEllipse(){
    EllipseShape * newEllipse = new EllipseShape;
    newEllipse->setPosition(
                         m_tagStack.back()->second.get<float>("<xmlattr>.cx"),
                         m_tagStack.back()->second.get<float>("<xmlattr>.cy") );
    newEllipse->setPosition(
                       m_tagStack.back()->second.get<float>("<xmlattr>.cx"),
                       m_tagStack.back()->second.get<float>("<xmlattr>.cy"));
    
    applyColorAsShape( newEllipse );
    return std::unique_ptr< sf::Drawable >( newEllipse );
    
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
        shape->setPoint( pCount-1, sf::Vector2f( x, y) );
    }
    applyColorAsShape( shape );
    
    returnPool.push_back(std::unique_ptr<sf::Drawable>( shape ));
    
    return std::move( returnPool );
}
    
return_t Loader::shapesFromSVGTag(){
    assert( !m_tagStack.empty() );
	return_t shapes;
	if( m_tagStack.back()->first == "rect" ){
		shapes.push_back( readAsRect() );
	} else if( m_tagStack.back()->first == "circle" ) {
		shapes.push_back( readAsCircle() );
	} else if( m_tagStack.back()->first == "ellipse" ) {
		shapes.push_back( readAsEllipse() );
	} else if( m_tagStack.back()->first == "line" ) {
		shapes.push_back( readAsLine() );
	} else if( m_tagStack.back()->first == "polyline" ) {
		shapes = readAsPolyline();
    } else if( m_tagStack.back()->first == "polygon" ) {
        shapes = readAsPolygon();
    }

	return std::move( shapes );
}

return_t Loader::read( const std::string &path ){
	
	return_t returnVector;

    pt::ptree tree;
    pt::read_xml(path, tree);

    for( auto &e : tree.get_child( "svg" ) )
        recursiveParse( e );

    return std::move( m_drawablePool );
}

void Loader::recursiveParse( pt::ptree::value_type &shapeNode ){
    std::cout << "parsing " << shapeNode.first << "\n";
    if( shapeNode.first == "<xmlattr>" ) //no need to worry about these
        return;
    if( shapeNode.first == "g" ){
        //group node
        //push it onto the stack so it's children can access it
        m_tagStack.push_back( &shapeNode );
        //send it's children through the parser
        for(pt::ptree::value_type &child : shapeNode.second ) {
            recursiveParse( child );
        }
        m_tagStack.pop_back(); // we're done with this group
        
    } else {
        //maybe some kind of shape node?
        try{
            
            m_tagStack.push_back( &shapeNode );
            auto shapes = shapesFromSVGTag();
            if( shapes.empty() )
                std::cout << "parsing tag " << shapeNode.first << " didn't produce anything.\n";
            for (auto &shape : shapes) {
                if( shape.get() != nullptr )
                    m_drawablePool.push_back( std::move( shape ) );
            }
            m_tagStack.pop_back();
        } catch( pt::ptree_bad_path e ){
            std::cout << "bad path = " << e.what()<< std::endl;
        }
    }
}
    
}//end of svg2sfml namespace
