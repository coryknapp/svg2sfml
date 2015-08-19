#include "svg2sfml.hpp"

#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <set>
#include <exception>

#include <math.h>

#include <SFML/Graphics.hpp>

#include "color_table.hpp"
#include <sfml-utilities/sfml-utilities.hpp>
namespace pt = boost::property_tree;
namespace svg2sfml{

static const std::string default_fill_color = "#FFFFFF";
static const std::string default_stroke_color = "#0000FF";

sf::Color colorFromSVGColor( const std::string &name ){

	if( name[0] == '#' ){
		std::cout << "color " << name << std::endl;
		int r = stoi( name.substr(1,2), nullptr, 16 );
		int g = stoi( name.substr(3,2), nullptr, 16 );
		int b = stoi( name.substr(5,2), nullptr, 16 );
		std::cout << r << ',' <<g<<','<<b << std::endl;
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

namespace make
{

void colorShape( sf::Shape * shape, pt::ptree::value_type &tag ){
//color/stroke attributes shared by all shapes
	shape->setFillColor( colorFromSVGColor(
			tag.second.get<std::string>("<xmlattr>.fill" , default_fill_color) ) );
	shape->setOutlineColor(
		colorFromSVGColor(
			tag.second.get<std::string>("<xmlattr>.stroke", default_stroke_color) ) );
	shape->setOutlineThickness(
		tag.second.get<float>("<xmlattr>.stroke-width", 0.0f) );
}

void colorLine( sf::Shape * shape, pt::ptree::value_type &tag ){
//color/stroke attributes shared by all shapes
	shape->setFillColor( colorFromSVGColor(
			tag.second.get<std::string>("<xmlattr>.stroke" , default_fill_color) ) );
	shape->setOutlineThickness( 0.0f );
}


std::unique_ptr< sf::Shape > rect( pt::ptree::value_type &tag ){
	sf::RectangleShape *newRect = new sf::RectangleShape;
		newRect->setPosition(
			tag.second.get<float>("<xmlattr>.x"),
			tag.second.get<float>("<xmlattr>.y") );
			std::cout << tag.second.get<float>("<xmlattr>.x") << std::endl;
		newRect->setSize( sf::Vector2f(
			tag.second.get<float>("<xmlattr>.width"),
			tag.second.get<float>("<xmlattr>.height") ) );
		//TODO support rounded rectangles
		//returnShape->setRoundedRectangles(
		//	tag.second.get<float>("rx"),
		//	tag.second.get<float>("ry") );
	colorShape( newRect, tag );
	return std::unique_ptr< sf::Shape >( newRect );
}

std::unique_ptr< sf::Shape > circle( pt::ptree::value_type &tag ){
	sf::CircleShape * newCirc = new sf::CircleShape;
	newCirc->setPosition(
		tag.second.get<float>("<xmlattr>.cx"),
		tag.second.get<float>("<xmlattr>.cy") );
	newCirc->setOrigin(
		tag.second.get<float>("<xmlattr>.r"),
		tag.second.get<float>("<xmlattr>.r"));

	newCirc->setRadius(
		tag.second.get<float>("<xmlattr>.r") );
	colorShape( newCirc, tag );
	return std::unique_ptr< sf::Shape >( newCirc );
}

std::unique_ptr< sf::Shape > ellipse( pt::ptree::value_type &tag ){

}

std::unique_ptr< sf::Shape >rectForLine(
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
	return std::unique_ptr< sf::Shape >( newRect );
}

std::unique_ptr< sf::Shape > line( pt::ptree::value_type &tag ){
	float x1 = tag.second.get<float>("<xmlattr>.x1");
	float y1 = tag.second.get<float>("<xmlattr>.y1");
	float x2 = tag.second.get<float>("<xmlattr>.x2");
	float y2 = tag.second.get<float>("<xmlattr>.y2");
	float width = tag.second.get<float>("<xmlattr>.stroke-width");
	auto rect = rectForLine( x1, y1, x2, y2, width );
	colorLine( rect.get(), tag );
	return rect;
}



std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

return_t polyline( pt::ptree::value_type &tag ){
	return_t returnList;
	std::string pointString = tag.second.get<std::string>("<xmlattr>.points");
	float width = tag.second.get<float>("<xmlattr>.stroke-width");
	std::vector<std::string> sets = split(pointString,' ');
	for(int n=0; n<sets.size()-1; n++ ){
		std::vector<std::string>set1 = split( sets[n], ',' );
		std::vector<std::string>set2 = split( sets[n+1], ',' );
		auto segment = rectForLine( stoi(set1[0]), stoi(set1[1]),
			stoi(set2[0]), stoi(set2[1]), width);
		colorLine( segment.get(), tag );
		returnList.push_back(std::move( segment ));
	}
	return returnList;
}
} /* make */ 



return_t shapesFromSVGTag( pt::ptree::value_type &tag ){
	std::cout << "tag = "<< tag.first << std::endl;
	return_t shapes;
	if( tag.first == "rect" ){
		shapes.push_back( make::rect( tag ) );
	} else if( tag.first == "circle" ) {
		shapes.push_back( make::circle( tag ) );
	} else if( tag.first == "ellipse" ) {
		shapes.push_back( make::ellipse( tag ) );
	} else if( tag.first == "line" ) {
		shapes.push_back( make::line( tag ) );
	} else if( tag.first == "polyline" ) {
		shapes = make::polyline( tag );
	}

	return std::move( shapes );
}

return_t readSVG( const std::string &path ){
	
	return_t returnVector;

    pt::ptree tree;
    pt::read_xml(path, tree);

    for(pt::ptree::value_type &shapeNode : tree.get_child("svg")) {
        // The data function is used to access the data stored in a node.
		try{
			//only needed to trigger the catch.  must be a better way
			auto &attributes =  shapeNode.second.get_child("<xmlattr>");
			auto shapes = shapesFromSVGTag( shapeNode );
			for (auto &shape : shapes) {
				if( shape.get() != nullptr )
					returnVector.push_back( std::move( shape ) );
			}
		} catch( pt::ptree_bad_path e ){
			std::cout << "bad path = " << e.what()<< std::endl;
		}
	}

	std::cout << "kkkk="<<returnVector.size() << std::endl;

	return returnVector;
}

}
