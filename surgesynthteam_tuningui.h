// -*-c++-*-
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 surgesynthteam_tuningui
  vendor:             surgesynthteam
  version:            1.0.0
  name:               Surge Synth Team JUCE UI Componenents for Tuning Synths
  description:        Various UI helpers for making rich tuning UIs
  website:            http://surge-synth-team.org/
  license:            GPL3

  dependencies:       juce_graphics juce_gui_basics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once

#include "Tunings.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <sstream>
#include <set>

namespace surgesynthteam
{
    
class TuningTableListBoxModel : public juce::TableListBoxModel,
                                               public juce::AsyncUpdater
{
public:
    TuningTableListBoxModel() {
        for( int i=0; i<128; ++i )
            notesOn[i] = false;
        
        rmbMenu = std::make_unique<juce::PopupMenu>();
    }
    ~TuningTableListBoxModel() {
        table = nullptr;
    }

    void setTableListBox( juce::TableListBox * t ) {
        table = t;
    }

// Column IDs.
// According to JUCE API (https://docs.juce.com/master/classTableHeaderComponent.html), 
// column IDs can be any number but _APART_ _FROM_ 0.
#define COLUMNID_NOTE 1
#define COLUMNID_NAME 2
#define COLUMNID_FREQ 3
#define COLUMNID_LOG2F 4
    void setupDefaultHeaders( juce::TableListBox *table ) {
        table->getHeader().addColumn( "Note", COLUMNID_NOTE, 40 );
        table->getHeader().addColumn( "Name", COLUMNID_NAME, 40 );
        table->getHeader().addColumn( "Freq (hz)", COLUMNID_FREQ, 90 );
        table->getHeader().addColumn( "log2(f/8.17)", COLUMNID_LOG2F, 90 );
    }
    
    virtual int getNumRows() override { return 128; }
    virtual void paintRowBackground( juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected ) override {
        if( ! table ) return;
        
        auto alternateColour = table->getLookAndFeel().findColour (juce::ListBox::backgroundColourId)
            .interpolatedWith (table->getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
        if( rowNumber % 2 )
            g.fillAll (alternateColour);
    }
    
    virtual void paintCell( juce::Graphics &g, int rowNumber, int columnID, int width, int height, bool rowIsSelected ) override {
        if( ! table ) return;
        
        int noteInScale = rowNumber % 12;
        bool whitekey = true;
        bool noblack = false;
        if( (noteInScale == 1 ||
             noteInScale == 3 ||
             noteInScale == 6 ||
             noteInScale == 8 ||
             noteInScale == 10 ) )
        {
            whitekey = false;
        }
        if( noteInScale == 4 || noteInScale == 11 )
            noblack = true;
        
        // Black Key
        auto kbdColour = table->getLookAndFeel().findColour( juce::ListBox::backgroundColourId );
        if( whitekey )
            kbdColour = kbdColour.interpolatedWith( table->getLookAndFeel().findColour( juce::ListBox::textColourId ), 0.3f );
        
        bool no = true;
        auto pressedColour = juce::Colour( 0xFFaaaa50 );
        
        if( notesOn[rowNumber] )
        {
            no = true;
            kbdColour = pressedColour;
        }
        
        g.fillAll( kbdColour );
        if( ! whitekey && columnID != COLUMNID_NOTE && no )
        {
            g.setColour (table->getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
            // draw an inset top and bottom
            g.fillRect (0, 0, width - 1, 1 );
            g.fillRect (0, height-1, width - 1, 1 );
        }
        
        int txtOff = 0;
        if( columnID == COLUMNID_NOTE )
        {
            // Black Key
            if( ! whitekey )
            {
                txtOff = 10;
                // "Black Key"
                auto kbdColour = table->getLookAndFeel().findColour( juce::ListBox::backgroundColourId );
                auto kbc = kbdColour.interpolatedWith( table->getLookAndFeel().findColour( juce::ListBox::textColourId ), 0.3f );
                g.setColour(kbc);
                g.fillRect(0,0,txtOff, height );
                
                // OK so now check neighbors
                if( rowNumber > 0 && notesOn[rowNumber - 1] )
                {
                    g.setColour(pressedColour);
                    g.fillRect (0, 0, txtOff, height / 2 );
                }
                if( rowNumber < 127 && notesOn[rowNumber + 1] )
                {
                    g.setColour(pressedColour);
                    g.fillRect (0, height / 2, txtOff, height / 2 );
                }
                g.setColour (table->getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
                g.fillRect (0, height / 2, txtOff, 1 );
                
                if( no )
                {
                    g.fillRect (txtOff, 0, width - 1 - txtOff, 1 );
                    g.fillRect (txtOff, height-1, width - 1 - txtOff, 1 );
                    g.fillRect (txtOff, 0, 1, height - 1 );
                }
            }
        }
        
        g.setColour ( table->getLookAndFeel().findColour (juce::ListBox::textColourId ) );
        
        auto mn = rowNumber;
        double lmn = tuning.logScaledFrequencyForMidiNote( mn );
        double fr  = tuning.frequencyForMidiNote( mn );
        
        char txt[256];
        
        switch( columnID ) {
        case COLUMNID_NOTE:
        {
            sprintf( txt, "%d", mn );
            break;
        }
        case COLUMNID_NAME:
        {
            static std::vector<std::string> nn = { { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" } };
            sprintf( txt, "%s%d", nn[noteInScale].c_str(), (int)(rowNumber / 12 ) - 1 );
            break;
        }
        case COLUMNID_FREQ:
        {
            sprintf( txt, "%.3lf", fr );
            break;
        }
        case COLUMNID_LOG2F:
        {
            sprintf( txt, "%.6lf", lmn );
            break;
        }
        }
        
        g.drawText (txt, 2 + txtOff, 0, width - 4, height, juce::Justification::centredLeft, true);
        g.setColour (table->getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
        g.fillRect (width - 1, 0, 1, height);
        if( noblack )
            g.fillRect (0, height - 1, width, 1 );
    }

    virtual void cellClicked (int rowNumber, int columnId, const juce::MouseEvent & e) override {
        if( e.mods.isRightButtonDown() )
        {
            rmbMenu->clear();
            rmbMenu->addItem(1, "Export to CSV" );
            auto result = rmbMenu->show();
            if( result == 1 )
                exportToCSV();
        }
    }

    virtual void exportToCSV() {
        juce::FileChooser fc( "Export CSV to...", juce::File(), "*.csv" );
        if( fc.browseForFileToSave(true) )
        {
            auto f = fc.getResult();
            std::ostringstream csvStream;
            csvStream << "Midi Note, Frequency, Log(Freq/8.17)\n";
            for( int i=0; i<128; ++i )
                csvStream << i << ", "
                          << std::fixed << std::setprecision( 4 ) << tuning.frequencyForMidiNote( i ) << ", "
                          << std::fixed << std::setprecision(6) << tuning.logScaledFrequencyForMidiNote( i ) << "\n";
            if( ! f.replaceWithText( csvStream.str() ) )
            {
                juce::AlertWindow::showMessageBoxAsync( juce::AlertWindow::AlertIconType::WarningIcon,
                                                        "Error exporting file",
                                                        "An unknown error occured streaming CSV data to file",
                                                        "OK" );

            }
        }
    }
       

    virtual void tuningUpdated( const Tunings::Tuning &newTuning )  {
        tuning = newTuning;
        triggerAsyncUpdate();
    }
    virtual void noteOn( int noteNum )  {
        notesOn[noteNum] = true;
        triggerAsyncUpdate();
    }
    virtual void noteOff( int noteNum )  {
        notesOn[noteNum] = false;
        triggerAsyncUpdate();
    }
    virtual void handleAsyncUpdate() override {
        if( table )
            table->repaint();
    }

private:
    Tunings::Tuning tuning;
    std::array<std::atomic<bool>, 128> notesOn;
    std::unique_ptr<juce::PopupMenu> rmbMenu;
    juce::TableListBox *table;

};


class ScaleEditor : public juce::Component, public juce::FileDragAndDropTarget
{
public:

    class ToneEditor : public juce::Component, public juce::TextEditor::Listener, public juce::Button::Listener {
    public:
        ToneEditor(bool editable);

        std::unique_ptr<juce::Label> displayIndex;
        std::unique_ptr<juce::TextEditor> displayValue;
        std::unique_ptr<juce::Component> coarseKnob, fineKnob, playingLED;
        std::unique_ptr<juce::TextButton> hideButton;

        ScaleEditor *parent;
        
        float cents;
        int index;

        int playingNotes = 0;
        void incNotes();
        void decNotes();
        
        virtual void 	textEditorTextChanged (juce::TextEditor &) override {
            onToneChanged(index, displayValue->getText());
        }

        virtual void buttonClicked(juce::Button *b) override;
        virtual void displayText( bool dt );
        
        std::function<void(int index, juce::String)> onToneChanged = [](int, juce::String) { };
    };

    class RadialScaleGraph;
    class GeneratorSection;
    
    ScaleEditor(Tunings::Scale &s);
    ~ScaleEditor() override;

    class ScaleTextEditedListener {
    public:
        virtual ~ScaleTextEditedListener() { }
        virtual void scaleTextEdited( juce::String newScale ) = 0;
    };
    void addScaleTextEditedListener( ScaleTextEditedListener *sel ) { listeners.insert(sel); }
    void removeScaleTextEditedListener( ScaleTextEditedListener *sel ) { listeners.erase(sel); }

    void recalculateScaleText();
    
    void setScaleText( juce::String &s ) {
        scaleText = s;
        scale = Tunings::parseSCLData(s.toStdString());
    }

    void buildUIFromScale();
    void toggleToneDisplay();
    bool displayTones = true;
    
    virtual bool isInterestedInFileDrag (const juce::StringArray& filenames) override {
        if( filenames.size() == 1 )
        {
            juce::File f( filenames[ 0 ] );
            if( f.hasFileExtension( ".scl" ) )
                return true;
        }
        return false;
    }

    virtual void filesDropped (const juce::StringArray& filenames, int mouseX, int mouseY) override {
        if( filenames.size() != 1 ) return;
        juce::File f( filenames[0] );
        if( f.hasFileExtension( ".scl" ) )
        {
            auto s = f.loadFileAsString();
            setScaleText( s );
            for( auto sl : listeners )
                sl->scaleTextEdited( s );
            buildUIFromScale();
        }
    }

    void resetScale( const Tunings::Scale &s )
        {
            if( scale.rawText != s.rawText )
            {
                scale = s;
                scaleText = s.rawText;
                buildUIFromScale();
                repaint();
            }
        }
    
    virtual void paint (juce::Graphics& g) override {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void scaleNoteOn( int scaleNote );
    void scaleNoteOff( int scaleNote );
        
private:
    std::set<ScaleTextEditedListener *> listeners;
    juce::String scaleText;
    Tunings::Scale scale;
    std::vector<std::unique_ptr<ToneEditor>> toneEditors;
    std::unique_ptr<juce::Component> notesSection;
    std::unique_ptr<juce::Viewport> notesViewport;

    std::unique_ptr<juce::TabbedComponent> analyticsTab;
    RadialScaleGraph* radialScaleGraph;

    std::unique_ptr<GeneratorSection> generatorSection;
    
    std::unique_ptr<juce::GroupComponent> notesGroup, generatorGroup, analyticsGroup;
};

class ScaleEditorWindow : public juce::DocumentWindow {
public:
    ScaleEditorWindow(ScaleEditor *s) // of which I take ownership
        : juce::DocumentWindow ("Advanced Scale Editor",
                                juce::Colour( 0xFF000000 ),
                                juce::DocumentWindow::allButtons ),
          editor( s )
        {
            if( editor )
            {
                int baroff = 25;
                addAndMakeVisible( editor );
                juce::Rectangle<int> area( 0, baroff,  editor->getWidth(), editor->getHeight() + baroff );

                juce::RectanglePlacement placement (juce::RectanglePlacement::xLeft
                                                    | juce::RectanglePlacement::yTop
                                                    | juce::RectanglePlacement::doNotResize);

                auto result = placement.appliedTo (area, juce::Desktop::getInstance().getDisplays()
                                                   .getPrimaryDisplay()->userArea.reduced (20));
                setBounds (result);
                editor->setBounds( 0, baroff, editor->getWidth(), editor->getHeight() );

                setResizable( false, false );
                setUsingNativeTitleBar( false );
            }
        }

    ~ScaleEditorWindow() {
        if( editor )
            delete editor;
    }
    void closeButtonPressed()
        {
            onCloseButton();
            delete this; // sigh.
        }

    std::function<void(void)> onCloseButton = [](){};
    ScaleEditor *editor;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScaleEditorWindow);
};


};
