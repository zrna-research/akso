package axoloti.swingui.patch.object.display;

import axoloti.patch.object.display.DisplayInstance;
import axoloti.patch.object.display.DisplayInstanceNoteLabel;
import axoloti.swingui.components.LabelComponent;
import java.beans.PropertyChangeEvent;
import axoloti.datatypes.Value;
import axoloti.datatypes.ValueFrac32;

class DisplayInstanceViewNoteLabel extends DisplayInstanceViewFrac32 {

    private LabelComponent readout;
    private DisplayInstanceNoteLabel displayInstance;

    DisplayInstanceViewNoteLabel(DisplayInstance displayInstance) {
        super(displayInstance);
        initComponents();
        this.displayInstance = (DisplayInstanceNoteLabel) displayInstance;
    }

    private void initComponents() {
        readout = new LabelComponent("xxxxx");
        add(readout);
        readout.setSize(40, 18);
    }

    @Override
    public DisplayInstanceNoteLabel getDModel() {
        return (DisplayInstanceNoteLabel) super.getDModel();
    }

    @Override
    public void modelPropertyChange(PropertyChangeEvent evt) {
        super.modelPropertyChange(evt);
        if (DisplayInstance.DISP_VALUE.is(evt)) {
            //throw new UnsupportedOperationException("Not supported yet.");

            readout.setText(displayInstance.getConv().convertToReal(new ValueFrac32((Double) evt.getNewValue())));
//            readout.setText("what the fuck\n");
        }
    }

}
