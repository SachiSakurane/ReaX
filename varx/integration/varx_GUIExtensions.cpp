using std::placeholders::_1;

ComponentExtension::ComponentExtension(Component& parent)
: parent(parent),
  visible(parent.isVisible())
{
    Array<PublishSubject<Colour>> colourSubjects;
    storeSubject = [colourSubjects](const PublishSubject<Colour>& subject) mutable { colourSubjects.add(subject); };

    parent.addComponentListener(this);
    visible.takeUntil(deallocated).subscribe(std::bind(&Component::setVisible, &parent, _1));
}

Observer<Colour> ComponentExtension::colour(int colourId) const
{
    PublishSubject<Colour> subject;

    subject.takeUntil(deallocated).subscribe([colourId, this](const Colour& colour) {
        this->parent.setColour(colourId, colour);
    });

    storeSubject(subject);
    return subject;
}

void ComponentExtension::componentVisibilityChanged(Component& component)
{
    if (component.isVisible() != visible.getLatestItem())
        visible.onNext(component.isVisible());
}


ButtonExtension::ButtonExtension(Button& parent)
: ComponentExtension(parent),
  clicked(_clicked),
  buttonState(parent.getState()),
  toggleState(parent.getToggleState()),
  text(_text),
  tooltip(_tooltip)
{
    parent.addListener(this);

    _text.takeUntil(deallocated).subscribe(std::bind(&Button::setButtonText, &parent, _1));
    _tooltip.takeUntil(deallocated).subscribe(std::bind(&Button::setTooltip, &parent, _1));
    buttonState.takeUntil(deallocated).subscribe(std::bind(&Button::setState, &parent, _1));
    toggleState.takeUntil(deallocated).subscribe([&parent](bool toggled) {
        parent.setToggleState(toggled, sendNotificationSync);
    });
}

void ButtonExtension::buttonClicked(Button*)
{
    _clicked.onNext(Empty());
}

void ButtonExtension::buttonStateChanged(Button* button)
{
    if (button->getState() != buttonState.getLatestItem())
        buttonState.onNext(button->getState());

    if (button->getToggleState() != toggleState.getLatestItem())
        toggleState.onNext(button->getToggleState());
}

ImageComponentExtension::ImageComponentExtension(ImageComponent& parent)
: ComponentExtension(parent),
  image(_image),
  imagePlacement(_imagePlacement)
{
    _image.takeUntil(deallocated).subscribe([&parent](const Image& image) {
        parent.setImage(image);
    });

    _imagePlacement.takeUntil(deallocated).subscribe(std::bind(&ImageComponent::setImagePlacement, &parent, _1));
}

LabelExtension::LabelExtension(Label& parent)
: ComponentExtension(parent),
  _discardChangesWhenHidingEditor(false),
  _textEditor(parent.getCurrentTextEditor()),
  text(parent.getText()),
  showEditor(parent.getCurrentTextEditor() != nullptr),
  discardChangesWhenHidingEditor(_discardChangesWhenHidingEditor),
  font(_font),
  justificationType(_justificationType),
  borderSize(_borderSize),
  attachedComponent(_attachedComponent),
  attachedOnLeft(_attachedOnLeft),
  minimumHorizontalScale(_minimumHorizontalScale),
  keyboardType(_keyboardType),
  editableOnSingleClick(_editableOnSingleClick),
  editableOnDoubleClick(_editableOnDoubleClick),
  lossOfFocusDiscardsChanges(_lossOfFocusDiscardsChanges),
  textEditor(_textEditor.distinctUntilChanged())
{
    parent.addListener(this);

    text.takeUntil(deallocated).subscribe(std::bind(&Label::setText, &parent, _1, sendNotificationSync));

    showEditor.withLatestFrom(_discardChangesWhenHidingEditor).takeUntil(deallocated).subscribe([&parent](const std::pair<bool, bool>& pair) {
        if (pair.first)
            parent.showEditor();
        else
            parent.hideEditor(pair.second);
    });

    _font.takeUntil(deallocated).subscribe(std::bind(&Label::setFont, &parent, _1));
    _justificationType.takeUntil(deallocated).subscribe(std::bind(&Label::setJustificationType, &parent, _1));
    _borderSize.takeUntil(deallocated).subscribe(std::bind(&Label::setBorderSize, &parent, _1));
    
    _attachedComponent.takeUntil(deallocated).subscribe([&parent](const WeakReference<Component>& component) {
        parent.attachToComponent(component, parent.isAttachedOnLeft());
    });
    
    _attachedOnLeft.takeUntil(deallocated).subscribe([&parent](bool attachedOnLeft) {
        parent.attachToComponent(parent.getAttachedComponent(), attachedOnLeft);
    });

    _minimumHorizontalScale.takeUntil(deallocated).subscribe(std::bind(&Label::setMinimumHorizontalScale, &parent, _1));

    _keyboardType.takeUntil(deallocated).subscribe([&parent](TextInputTarget::VirtualKeyboardType keyboardType) {
        parent.setKeyboardType(keyboardType);

        if (auto editor = parent.getCurrentTextEditor())
            editor->setKeyboardType(keyboardType);
    });

    _editableOnSingleClick.combineLatest(_editableOnDoubleClick, _lossOfFocusDiscardsChanges).skip(1).takeUntil(deallocated).subscribe([&parent](const std::tuple<bool, bool, bool>& tuple) {
        parent.setEditable(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
    });
    
    // Cannot use combineLatest for these, because changing something on the Slider directly doesn't update the subject
    _editableOnSingleClick.takeUntil(deallocated).subscribe([&parent](bool editableOnSingleClick) {
        parent.setEditable(editableOnSingleClick, parent.isEditableOnDoubleClick(), parent.doesLossOfFocusDiscardChanges());
    });
    _editableOnDoubleClick.takeUntil(deallocated).subscribe([&parent](bool editableOnDoubleClick) {
        parent.setEditable(parent.isEditableOnSingleClick(), editableOnDoubleClick, parent.doesLossOfFocusDiscardChanges());
    });
    _lossOfFocusDiscardsChanges.takeUntil(deallocated).subscribe([&parent](bool lossOfFocusDiscardsChanges) {
        parent.setEditable(parent.isEditableOnSingleClick(), parent.isEditableOnDoubleClick(), lossOfFocusDiscardsChanges);
    });
}

void LabelExtension::labelTextChanged(Label* parent)
{
    if (parent->getText() != text.getLatestItem())
        text.onNext(parent->getText());
}

void LabelExtension::editorShown(Label* parent, TextEditor&)
{
    if (!showEditor.getLatestItem())
        showEditor.onNext(true);

    _textEditor.onNext(parent->getCurrentTextEditor());
}

void LabelExtension::editorHidden(Label* parent, TextEditor&)
{
    if (showEditor.getLatestItem())
        showEditor.onNext(false);

    _textEditor.onNext(parent->getCurrentTextEditor());
}


SliderExtension::SliderExtension(juce::Slider& parent, const Observer<std::function<double(const juce::String&)>>& getValueFromText, const Observer<std::function<juce::String(double)>>& getTextFromValue)
: ComponentExtension(parent),
  _dragging(false),
  _discardChangesWhenHidingTextBox(false),
  value(parent.getValue()),
  minimum(_minimum),
  maximum(_maximum),
  minValue(hasMultipleThumbs(parent) ? parent.getMinValue() : parent.getValue()),
  maxValue(hasMultipleThumbs(parent) ? parent.getMaxValue() : parent.getValue()),
  doubleClickReturnValue(_doubleClickReturnValue),
  interval(_interval),
  skewFactorMidPoint(_skewFactorMidPoint),
  dragging(_dragging.distinctUntilChanged()),
  thumbBeingDragged(dragging.map([&parent](bool) { return parent.getThumbBeingDragged(); })),
  showTextBox(_showTextBox),
  textBoxIsEditable(_textBoxIsEditable),
  discardChangesWhenHidingTextBox(_discardChangesWhenHidingTextBox),
  getValueFromText(getValueFromText),
  getTextFromValue(getTextFromValue)
{
    parent.addListener(this);

    value.takeUntil(deallocated).subscribe([&parent](double value) {
        parent.setValue(value, sendNotificationSync);
    });
    
    // Cannot use combineLatest for these, because changing something on the Slider directly doesn't update the subject
    _minimum.takeUntil(deallocated).subscribe([&parent](double minimum) {
        parent.setRange(minimum, parent.getMaximum(), parent.getInterval());
    });
    _maximum.takeUntil(deallocated).subscribe([&parent](double maximum) {
        parent.setRange(parent.getMinimum(), maximum, parent.getInterval());
    });
    _interval.takeUntil(deallocated).subscribe([&parent](double interval) {
        parent.setRange(parent.getMinimum(), parent.getMaximum(), interval);
    });

    minValue.skip(1).takeUntil(deallocated).subscribe([&parent](double minValue) {
        parent.setMinValue(minValue, sendNotificationSync, true);
    });
    
    maxValue.skip(1).takeUntil(deallocated).subscribe([&parent](double maxValue) {
        parent.setMaxValue(maxValue, sendNotificationSync, true);
    });

    _doubleClickReturnValue.takeUntil(deallocated).subscribe([&parent](double value) {
        parent.setDoubleClickReturnValue(value != std::numeric_limits<double>::max(), value);
    });

    _skewFactorMidPoint.takeUntil(deallocated).subscribe(std::bind(&Slider::setSkewFactorFromMidPoint, &parent, _1));

    _showTextBox.withLatestFrom(_discardChangesWhenHidingTextBox).takeUntil(deallocated).subscribe([&parent](const std::pair<bool, bool>& pair) {
        if (pair.first)
            parent.showTextBox();
        else
            parent.hideTextBox(pair.second);
    });

    _textBoxIsEditable.takeUntil(deallocated).subscribe(std::bind(&Slider::setTextBoxIsEditable, &parent, _1));
}

void SliderExtension::sliderValueChanged(Slider* slider)
{
    if (slider->getValue() != value.getLatestItem())
        value.onNext(slider->getValue());

    if (hasMultipleThumbs(*slider) && slider->getMinValue() != minValue.getLatestItem())
        minValue.onNext(slider->getMinValue());

    if (hasMultipleThumbs(*slider) && slider->getMaxValue() != maxValue.getLatestItem())
        maxValue.onNext(slider->getMaxValue());
}

void SliderExtension::sliderDragStarted(Slider*)
{
    _dragging.onNext(true);
}

void SliderExtension::sliderDragEnded(Slider*)
{
    _dragging.onNext(false);
}

bool SliderExtension::hasMultipleThumbs(const juce::Slider& parent)
{
    switch (parent.getSliderStyle()) {
        case Slider::TwoValueHorizontal:
        case Slider::TwoValueVertical:
        case Slider::ThreeValueHorizontal:
        case Slider::ThreeValueVertical:
            return true;

        default:
            return false;
    }
}
