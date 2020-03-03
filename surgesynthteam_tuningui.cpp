#include "surgesynthteam_tuningui.h"

using namespace juce;


surgesynthteam_ScaleEditor::ToneEditor::ToneEditor()
{
    displayValue.reset( new TextEditor( "display value" ) );
    addAndMakeVisible( displayValue.get() );
    displayValue->setBounds( 2, 2, 100, 20 );
    
    cents.reset( new Label( "cents" ) );
    addAndMakeVisible( cents.get() );
    cents->setBounds( 2, 102, 80, 20 );
    
    setSize( 300, 24 );
}

surgesynthteam_ScaleEditor::surgesynthteam_ScaleEditor() {
    std::cout << "Made a scale editor" << std::endl;

    scale = Tunings::evenTemperament12NoteScale();
    scaleText = scale.rawText;

    buildUIFromScale();
    
    setSize( 800, 600 );
}

surgesynthteam_ScaleEditor::~surgesynthteam_ScaleEditor() {
}


void surgesynthteam_ScaleEditor::buildUIFromScale()
{
    if( notesSection == nullptr )
    {
        // It's my first time through
        notesSection = std::make_unique<Component>();
        notesViewport = std::make_unique<Viewport>();
        notesViewport->setViewedComponent( notesSection.get(), false );
        notesViewport->setBounds( 10, 100, 380, 490 );
        notesViewport->setScrollBarsShown( true, false );
        addAndMakeVisible( notesViewport.get() );
    }

    // clear prior components
    for( auto &p : toneEditors )
        notesSection->removeChildComponent( p.get() );
    toneEditors.clear();

    int nEds = scale.count * 4;
    for( int i=0; i<nEds; ++i )
    {
        auto t = std::make_unique<ToneEditor>();
        notesSection->addAndMakeVisible( t.get() );
        t->setBounds( 10, 28 * i, 200, 24 );
        t->displayValue->setText( juce::String( std::to_string( i ) ), dontSendNotification );
        
        toneEditors.push_back( std::move( t ) );
    }
    notesSection->setSize( 380, 30 * (nEds + 1 ) );
}
