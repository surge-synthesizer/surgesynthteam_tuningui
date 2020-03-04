#include "surgesynthteam_tuningui.h"

using namespace juce;


surgesynthteam_ScaleEditor::ToneEditor::ToneEditor()
{
    int xpos = 2;
    index.reset( new Label( "idx" ) );
    addAndMakeVisible( index.get() );
    index->setBounds( xpos, 2, 40, 20 );
    xpos += 44;
    
    displayValue.reset( new TextEditor( "display value" ) );
    addAndMakeVisible( displayValue.get() );
    displayValue->setBounds( xpos, 2, 100, 20 );
    xpos += 104;
    
    cents.reset( new Label( "cents" ) );
    addAndMakeVisible( cents.get() );
    cents->setBounds( xpos, 2, 80, 20 );
    xpos += 84;
    
    setSize( 300, 24 );
}

surgesynthteam_ScaleEditor::surgesynthteam_ScaleEditor(Tunings::Scale &inScale) {
    scale = inScale;
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
        countDescGroup.reset( new juce::GroupComponent( "cdg", TRANS( "Count and Description" ) ) );
        addAndMakeVisible( countDescGroup.get() );
        countDescGroup->setBounds( 4, 4, 394, 96 );

        notesGroup.reset( new juce::GroupComponent( "st", TRANS( "Scale Tones" ) ) );
        addAndMakeVisible( notesGroup.get() );
        notesGroup->setBounds( 4, 104, 394, 492 );

        generatorGroup.reset( new juce::GroupComponent( "cdg", TRANS( "Scale Generators" ) ) );
        addAndMakeVisible( generatorGroup.get() );
        generatorGroup->setBounds( 402, 4, 394, 96 );

        analyticsGroup.reset( new juce::GroupComponent( "st", TRANS( "Analytics" ) ) );
        addAndMakeVisible( analyticsGroup.get() );
        analyticsGroup->setBounds( 402, 104, 394, 492 );

        notesSection = std::make_unique<Component>();
        notesViewport = std::make_unique<Viewport>();
        notesViewport->setViewedComponent( notesSection.get(), false );
        notesViewport->setBounds( 10, 124, 380, 468 );
        notesViewport->setScrollBarsShown( true, false );
        addAndMakeVisible( notesViewport.get() );

        radialScaleGraph.reset( new RadialScaleGraph( scale ) );
        addAndMakeVisible(radialScaleGraph.get() );
        radialScaleGraph->setBounds( 410, 124, 380, 468 );
    }

    // clear prior components. Only need to do this if size has changed though. FIXME
    for( auto &p : toneEditors )
        notesSection->removeChildComponent( p.get() );
    toneEditors.clear();

    int nEds = scale.count;
    for( int i=0; i<nEds + 1; ++i )
    {
        auto t = std::make_unique<ToneEditor>();
        notesSection->addAndMakeVisible( t.get() );
        t->setBounds( 10, 28 * i, 200, 24 );

        if( i == 0 )
        {
            t->displayValue->setText( "1", dontSendNotification ); // make it all readonly too
            t->index->setText( "0", dontSendNotification );
        }
        else
        {
            auto tn = scale.tones[i-1];
            if( tn.type == Tunings::Tone::kToneRatio )
            {
                t->displayValue->setText( juce::String( std::to_string( tn.ratio_n ) + "/" + std::to_string( tn.ratio_d ) ) );
            }
            else
            {
                t->displayValue->setText( juce::String( std::to_string( tn.cents ) ), dontSendNotification );
            }
            t->index->setText( juce::String( std::to_string( i ) ), dontSendNotification );
        }
        
        toneEditors.push_back( std::move( t ) );
    }
    notesSection->setSize( 380, 30 * (nEds + 1 ) );
}

void surgesynthteam_ScaleEditor::RadialScaleGraph::paint( Graphics &g ) {
    // g.fillAll( Colour( 255, 0, 0 ) );
    int w = getWidth();
    int h = getHeight();
    float r = std::min( w, h )  / 2.0;
    float xo = ( w - 2 * r ) / 2.0;
    float yo = ( h - 2 * r ) / 2.0;

    g.saveState();

    g.addTransform( AffineTransform::translation( xo, yo ) );
    g.addTransform( AffineTransform::translation( r, r ) );
    g.addTransform( AffineTransform::scale( r, - r ) );
    g.addTransform( AffineTransform::scale( 0.7, 0.7 ) );

    // We are now in a normal x y 0 1 coordinate system

    // Now draw the gray spokes
    float op = 0.4;
    int nup = 2;
    int ndn = 3;
    for( int i=-ndn; i<=nup; ++i )
    {
        if( i == 0 )
        {
        }
        else
        {
            float pos = log(  std::abs( i ) + 1 ) / log(5 + 1);

            pos = std::abs( i ) / nup;
            float cpos = std::max( -1.f, pos );
            
            g.setColour( Colour( 110, 110, 120 ).interpolatedWith( getLookAndFeel().findColour (ResizableWindow::backgroundColourId), cpos * 0.8 ) );

            float rad = 1;
            if( i < 0 )
                rad = 1.0 - pos * op;
            else
                rad = 1.0 + pos * op;
            g.drawEllipse( - rad, -rad, 2 * rad, 2 * rad, 0.01 );
        }
    }

    g.setColour( Colour( 110, 110, 120 ) );
    for( int i=0; i<scale.count; ++i )
    {
        double frac = 1.0 * i / (scale.count);
        double sx = std::sin( frac * 2.0 * MathConstants<double>::pi );
        double cx = std::cos( frac * 2.0 * MathConstants<double>::pi );

        g.drawLine( 0, 0, (1.0 + op) * sx, (1.0 + op) * cx, 0.01 );
    }

    // Draw the ring at 1.0
    g.setColour( Colour( 255, 255, 255 ) );
    g.drawEllipse( -1, -1, 2, 2, 0.01 );

    for( int i=-1; i<scale.count; ++i )
    {
        double frac = 1.0 * i / (scale.count);
        double sx = std::sin( frac * 2.0 * MathConstants<double>::pi );
        double cx = std::cos( frac * 2.0 * MathConstants<double>::pi );

        float rx = 1.0;
        if( i != -1 )
        {
            auto t = scale.tones[i];
            auto c = t.cents;
            auto expectedC = scale.tones.back().cents / scale.count;
            std::cout << i << " " << c << " " << expectedC * ( i + 1 ) << " " << ( c - expectedC * ( i + 1 ) )<< std::endl;
            
            rx = 1.0 + op / nup * ( c - expectedC * ( i + 1 ) ) / expectedC;

        }
        g.setColour( Colour( 255, 128 * rx, 0 ) );
        g.fillEllipse( rx * sx - 0.05, rx * cx - 0.05, 0.1, 0.1 );
    }

    g.restoreState();
}
