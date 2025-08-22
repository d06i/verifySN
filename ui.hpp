#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

enum MessageTypes : uint8_t{
   empty,
   normal,
   invalid,
   warning,
   success
};
  
class UI {
private:

	void render( Element element ) const {
		auto screen = Screen::Create(
			Dimension::Full(),
			Dimension::Fit(element)
		);

		Render(screen, element);
		screen.Print();

	}
	  
public:

	void setElement( const MessageTypes& type, const std::string& hash, const std::string filename ) const {
		
		Element temp;

		switch (type) {
			case MessageTypes::normal :
				temp = hbox({
					text(hash)	   | border | color(Color::Cyan),
					text(filename) | border | color(Color::Cyan),
					})	 ;
				break;
			case MessageTypes::empty:
				temp = hbox({
					text(hash)	   | border | color(Color::Khaki1),
					text(filename) | border | color(Color::Cyan),
					}) ;
				break;
			case MessageTypes::invalid:
				temp = hbox({
					text(hash)	   | border | color(Color::Red),
					text(filename) | border | color(Color::Red),
					});
				break;
			case MessageTypes::warning:
				temp = hbox({
					text(hash)	| border | color(Color::Khaki3),
					text(filename) | border | color(Color::Khaki1),
					});
				break;
			case MessageTypes::success:
				temp = hbox({
					text(hash)	   | border | color(Color::Green),
					text(filename) | border | color(Color::Green),
					});
				break;
		}

		render(temp);
		 
	}
	 
	void usage() const {

		auto element = vbox({
				window(text("Usage"),
					vbox({
					  text("VerifySN") | center,
					  text("by") | center,
					  text("d06i") | center,
					  separator(),
					  text("Usage: program.exe [options] filepath.") | center,
					  text("--save or -s : save hashes to hash.txt.") ,
					  text("--compare or -c : compare hash.") ,
					}), ftxui::DOUBLE
				)
			}) | center | color(Color::Khaki3);

		render(element);

	}

	void infos( const std::string& str1, const std::string& str2 ) const {

		auto element = vbox({
	    	window( text("INFO"), 
			  hbox({
					vtext("INFO") |center,
					separator(),
					vbox({
					  text(str1),
					  separatorDouble(),
					  text(str2)
					}) | flex
				})
			)	
		}) | center | color(Color::DarkOrange) ;




		render(element); 

	}
		 
};