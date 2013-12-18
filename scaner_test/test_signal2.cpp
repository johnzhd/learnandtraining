#include "stdafx.h"

#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <string>

class Document
{
public:
    typedef boost::signals2::signal<void ()>  signal_t;

public:
    Document()
    {}

    /* Connect a slot to the signal which will be emitted whenever
      text is appended to the document. */
    boost::signals2::connection connect(const signal_t::slot_type &subscriber)
    {
        return m_sig.connect(subscriber);
    }

    void append(const char* s)
    {
        m_text += s;
        m_sig();
    }

    const std::string& getText() const
    {
        return m_text;
    }

private:
    signal_t    m_sig;
    std::string m_text;
};

class TextView
{
public:
	 void refresh() const
    {
        std::cout << "TextView: " << m_document.getText() << std::endl;
    }
    TextView(Document& doc): m_document(doc)
    {
        m_connection = m_document.connect(boost::bind(&TextView::refresh, this));
    }

    ~TextView()
    {
        m_connection.disconnect();
    }

   
private:
    Document&               m_document;
    boost::signals2::connection  m_connection;
};


class HexView
{
public:
	void refresh() const
    {
        const std::string&  s = m_document.getText();

        std::cout << "HexView:";

        for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
            std::cout << ' ' << std::hex << static_cast<int>(*it);

        std::cout << std::endl;
    }
    HexView(Document& doc): m_document(doc)
    {
        m_connection = m_document.connect(boost::bind(&HexView::refresh, this));
    }

    ~HexView()
    {
        m_connection.disconnect();
    }

    
private:
    Document&               m_document;
    boost::signals2::connection  m_connection;
};


int main(int argc, char* argv[])
{
    Document    doc;
    TextView    v1(doc);
    HexView     v2(doc);

    doc.append(argc == 2 ? argv[1] : "Hello world!");
    return 0;
}
