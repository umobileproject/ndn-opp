/**
 *  @version 1.0
 * COPYRIGHTS COPELABS/ULHT, LGPLv3.0, 2017-02-14
 * Simple Dialog to Add a route to the ForwardingDaemon's RIB.
 * @author Seweryn Dynerowicz (COPELABS/ULHT)
 */

package pt.ulusofona.copelabs.ndn.android.ui.dialog;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;

import java.util.ArrayList;
import java.util.List;

import pt.ulusofona.copelabs.ndn.R;
import pt.ulusofona.copelabs.ndn.android.models.Face;
import pt.ulusofona.copelabs.ndn.android.umobile.OpportunisticDaemon;

/** Dialog for the addition of a new Route to the RIB and FIB of the running daemon.
 */
public class AddRouteDialog extends DialogFragment {
	private EditText mPrefix;
	private Spinner mFaces;

	/** Method to be used for creating a new AddRouteDialog.
	 * @param binder used to access the locally running daemon
	 * @return the AddRouteDialog
	 */
	public static AddRouteDialog create(OpportunisticDaemon.NodBinder binder) {
		AddRouteDialog fragment = new AddRouteDialog();
		Bundle args = new Bundle();
		args.putBinder("ForwardingDaemon", binder);
		fragment.setArguments(args);
		return fragment;
	}

	/** Constructs a dialog window which enables the addition of routes. Three parameters are requested through its fields;
	 * the Name prefix, the Face ID and the Cost.
	 * @param savedInstanceState used by Android for restoring the dialog object from a previous instance
	 * @return the Dialog to be displayed for adding a route.
	 */
	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		final OpportunisticDaemon.NodBinder fwdDaemon = (OpportunisticDaemon.NodBinder) getArguments().getBinder("ForwardingDaemon");

		View dialog = View.inflate(getContext(), R.layout.dialog_add_route, null);

		mPrefix = (EditText) dialog.findViewById(R.id.prefix);
		mFaces = (Spinner) dialog.findViewById(R.id.faces);
        List<String> spinnerList = new ArrayList<>();
        for(Face current : fwdDaemon.getFaceTable())
            spinnerList.add(Long.toString(current.getFaceId()));
        mFaces.setAdapter(new ArrayAdapter<>(getContext(), android.R.layout.simple_spinner_item, spinnerList));

		return builder
			.setView(dialog)
			.setPositiveButton(R.string.create, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface di, int id) {
				String host = mPrefix.getText().toString();
				String faceId = mFaces.getSelectedItem().toString();
				if(host.isEmpty())
					host = getString(R.string.defaultPrefix);
				if(faceId.isEmpty())
					faceId = "0";
				fwdDaemon.addRoute(host, Long.decode(faceId), 0L, 0L, 1L);
				}
			})
			.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface di, int id) {
					getDialog().cancel();
				}
			})
			.create();
	}
}