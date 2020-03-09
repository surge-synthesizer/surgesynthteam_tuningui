#include "surgesynthteam_tuningui.h"

using namespace juce;

namespace surgesynthteam
{
    
class InfiniteKnob : public Component {
public:
    InfiniteKnob() : Component(), angle(0) { }

    virtual void mouseDown (const MouseEvent &event) override {
        lastDrag = 0;
        isDragging= true;
        repaint();
    }
    virtual void mouseDrag (const MouseEvent &event) override {
        int d = - ( event.getDistanceFromDragStartX() + event.getDistanceFromDragStartY() );
        int diff = d - lastDrag;
        lastDrag = d;
        if( diff != 0 )
        {
            float mul = 1.0;
            if( event.mods.isShiftDown() )
            {
                mul = 0.05;
            }
            angle += diff * mul;
            onDragDelta(diff * mul);
            repaint();
        }
    }
    virtual void mouseUp (const MouseEvent &event) override {
        isDragging= false;
        repaint();
    }
    virtual void paint( Graphics &g ) override {
        g.saveState();
        g.addTransform( AffineTransform::translation( 10, 10 ) );
        g.addTransform( AffineTransform::rotation( angle / 50.0 * 2.0 * MathConstants<double>::pi ) );
        g.setColour( getLookAndFeel().findColour( juce::Slider::rotarySliderFillColourId ) );
        g.fillEllipse( -7, -7, 14, 14 );
        g.setColour( getLookAndFeel().findColour( juce::GroupComponent::outlineColourId ) );
        g.drawEllipse( -7, -7, 14, 14, 2 );
        g.setColour( getLookAndFeel().findColour( juce::Slider::thumbColourId ) );
        g.drawLine( 0, -9, 0, 9, 3 );
        g.restoreState();
    }

    int lastDrag = 0;
    bool isDragging = false;
    float angle;
    std::function<void(float)> onDragDelta = [](float f){};
};

ScaleEditor::ToneEditor::ToneEditor(bool editable)
{
    int xpos = 2;
    displayIndex.reset( new Label( "idx" ) );
    addAndMakeVisible( displayIndex.get() );
    displayIndex->setBounds( xpos, 2, 40, 20 );
    xpos += 44;
    
    displayValue.reset( new TextEditor( "display value" ) );
    addAndMakeVisible( displayValue.get() );
    displayValue->setBounds( xpos, 2, 100, 20 );
    displayValue->addListener( this );
    displayValue->setReadOnly( ! editable );
    xpos += 104;

    if( editable )
    {
        auto ck = new InfiniteKnob();
        ck->onDragDelta = [this](float f) {
                              auto nv = this->cents + f;
                              displayValue->setText( String( nv, 5 ) );
                          };
        addAndMakeVisible(ck);
        coarseKnob.reset( ck );
        coarseKnob->setBounds( xpos, 2, 20, 20 );
        xpos += 24;
        
        auto fk = new InfiniteKnob();
        addAndMakeVisible(fk);
        fk->onDragDelta = [this](float f) {
                              auto nv = this->cents + f * 0.05 * 0.1;
                              displayValue->setText( String( nv, 5 ) );
                          };

        fineKnob.reset( fk );
        fineKnob->setBounds( xpos, 2, 20, 20 );
        xpos += 24;
    }
    setSize( 300, 24 );
}

ScaleEditor::ScaleEditor(Tunings::Scale &inScale) {
    scale = inScale;
    scaleText = scale.rawText;

    buildUIFromScale();
    
    setSize( 800, 600 );
}

ScaleEditor::~ScaleEditor() {
}


void ScaleEditor::buildUIFromScale()
{
    if( notesSection == nullptr )
    {
        // It's my first time through<
        countDescGroup.reset( new juce::GroupComponent( "cdg", TRANS( "Count and Description" ) ) );
        addAndMakeVisible( countDescGroup.get() );
        countDescGroup->setBounds( 4, 4, 394, 96 );

        notesGroup.reset( new juce::GroupComponent( "st", TRANS( "Scale Tones" ) ) );
        addAndMakeVisible( notesGroup.get() );
        notesGroup->setBounds( 4, 104, 294, 492 );

        generatorGroup.reset( new juce::GroupComponent( "cdg", TRANS( "Scale Generators" ) ) );
        addAndMakeVisible( generatorGroup.get() );
        generatorGroup->setBounds( 402, 4, 394, 96 );

        notesSection = std::make_unique<Component>();
        notesViewport = std::make_unique<Viewport>();
        notesViewport->setViewedComponent( notesSection.get(), false );
        notesViewport->setBounds( 10, 124, 280, 468 );
        notesViewport->setScrollBarsShown( true, false );
        addAndMakeVisible( notesViewport.get() );

        analyticsGroup.reset( new juce::GroupComponent( "st", TRANS( "Analytics" ) ) );
        addAndMakeVisible( analyticsGroup.get() );
        analyticsGroup->setBounds( 302, 104, 494, 492 );

        analyticsTab.reset( new juce::TabbedComponent(TabbedButtonBar::TabsAtTop) );
        addAndMakeVisible( analyticsTab.get() );
        analyticsTab->setBounds( 310, 124, 480, 468 );
        analyticsTab->setTabBarDepth(30);
        radialScaleGraph = new RadialScaleGraph(scale);
        analyticsTab->addTab( TRANS( "Radial Graph" ), Colours::lightgrey, radialScaleGraph, true );
    }

    radialScaleGraph->scale = scale;
    
    // clear prior components. Only need to do this if size has changed though. FIXME
    int nEds = scale.count;
    if( toneEditors.size() != scale.count + 1 )
    {
        for( auto &p : toneEditors )
            notesSection->removeChildComponent( p.get() );
        toneEditors.clear();
        for( int i=0; i<nEds + 1; ++i )
        {
            auto t = std::make_unique<ToneEditor>( i != 0 );
            notesSection->addAndMakeVisible( t.get() );
            t->setBounds( 10, 28 * i, 200, 24 );
            toneEditors.push_back( std::move( t ) );
        }
    }

    for( int i=0; i<nEds + 1; ++i )
    {
        auto t = toneEditors[i].get();
        if( i == 0 )
        {
            t->displayValue->setText( "1", dontSendNotification ); // make it all readonly too
            t->displayIndex->setText( "0", dontSendNotification );
            t->index = 0;
            t->cents = 0;
        }
        else
        {
            auto tn = scale.tones[i-1];
            if( tn.type == Tunings::Tone::kToneRatio )
            {
                t->displayValue->setText( juce::String( std::to_string( tn.ratio_n ) + "/" + std::to_string( tn.ratio_d ) ), dontSendNotification );
            }
            else
            {
                t->displayValue->setText( juce::String( std::to_string( tn.cents ) ), dontSendNotification );
            }
            t->displayIndex->setText( juce::String( std::to_string( i ) ), dontSendNotification );
            t->index = i;
            t->cents = tn.cents;
            t->onToneChanged = [this]( int idx, juce::String newVal ) {
                                   try {
                                       this->scale.tones[idx-1] = Tunings::toneFromString( newVal.toStdString() );
                                       this->toneEditors[idx]->cents = this->scale.tones[idx-1].cents;
                                       this->recalculateScaleText();
                                   } catch( Tunings::TuningError &e ) {
                                       // Set background to red
                                       std::cout << "Tuning Error " << e.what() << std::endl;
                                   }
                               };
        }
    }
    notesSection->setSize( 380, 30 * (nEds + 1 ) );

    repaint();
}

void ScaleEditor::RadialScaleGraph::paint( Graphics &g ) {
    g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ) );
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
            
            rx = 1.0 + op / nup * ( c - expectedC * ( i + 1 ) ) / expectedC;

        }
        g.setColour( Colour( 255, 128 * rx, 0 ) );
        g.fillEllipse( rx * sx - 0.05, rx * cx - 0.05, 0.1, 0.1 );
    }

    g.restoreState();
}

void ScaleEditor::recalculateScaleText()
{
    std::ostringstream oss;
    oss << "! Scale generated by tuning editor\n"
        << "Description FIXME\n"
        << scale.count << "\n"
        << "! \n";
    for( int i=0; i<scale.count; ++i )
    {
        auto tn = scale.tones[i];
        if( tn.type == Tunings::Tone::kToneRatio )
        {
            oss << tn.ratio_n << "/" << tn.ratio_d << "\n";
        }
        else
        {
            oss << std::fixed << std::setprecision(5) << tn.cents << "\n";
        }
    }

    juce::String ns( oss.str().c_str() ); // sort of a dumb set of copies here. FIXME references and stuff
    for( auto sl : listeners )
        sl->scaleTextEdited( ns );

    radialScaleGraph->scale = scale;
    radialScaleGraph->repaint();
}

};
