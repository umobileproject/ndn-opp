/**
 *  @version 1.0
 * COPYRIGHTS COPELABS/ULHT, LGPLv3.0, 2017-02-14
 * This class manages the Fragment which displays the PeerTracking and controls Group Formation.
 * @author Seweryn Dynerowicz (COPELABS/ULHT)
 */
package pt.ulusofona.copelabs.ndn.android.ui.fragment;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Bundle;
import android.os.Looper;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;

import java.util.HashMap;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;

import pt.ulusofona.copelabs.ndn.R;
import pt.ulusofona.copelabs.ndn.android.models.NsdService;
import pt.ulusofona.copelabs.ndn.android.ui.adapter.NsdServiceAdapter;
import pt.ulusofona.copelabs.ndn.android.ui.adapter.WifiP2pPeerAdapter;
import pt.ulusofona.copelabs.ndn.android.umobile.Utilities;
import pt.ulusofona.copelabs.ndn.android.umobile.nsd.NsdServiceDiscoverer;
import pt.ulusofona.copelabs.ndn.android.umobile.wifip2p.OpportunisticConnectivityManager;
import pt.ulusofona.copelabs.ndn.android.umobile.wifip2p.OpportunisticPeer;
import pt.ulusofona.copelabs.ndn.android.umobile.wifip2p.OpportunisticPeerTracker;

/** Interface to the Peer Tracking functionality of NDN-Opp. This Fragment is responsible for integrating
 * the functionalities of the NsdServiceTracker, the WifiP2pPeerTracker and the WifiP2pConnectivityManager.
 *
 * The interactions between these three components is as follows;
 *
 * - The Peer Tracker provides the up-to-date list of Wi-Fi P2P devices running NDN-Opp that were encountered
 * - The Connectivity Manager is used to take care of the formation of a Wi-Fi Direct Group (whether to form a new one or join an existing one)
 * - The NSD Service Tracker is used to know which NDN-Opp daemon can be reached within the Group to which the current device is connected (if it is)
 */
public class OpportunisticPeerTracking extends Fragment implements Observer {
    private static final String TAG = OpportunisticPeerTracking.class.getSimpleName();

    // Used for feedback to the user that a peerDiscovery is in progress
    private ProgressBar mDiscoveryInProgress;
    // Used to detect changes to the peerDiscovery process (see WIFI_P2P_DISCOVERY_CHANGED_ACTION)
    private DiscoveryDetector mDiscoveryDetector = new DiscoveryDetector();

    private NsdServiceDiscoverer mServiceTracker = NsdServiceDiscoverer.getInstance();
    private OpportunisticPeerTracker mWifiP2pPeerTracker = OpportunisticPeerTracker.getInstance();
    private OpportunisticConnectivityManager mWifiP2pConnectivityManager = new OpportunisticConnectivityManager();

    // Two variables to remember whether to Group-related buttons have to be enabled or disabled.
    private boolean mCanFormGroup = false; // = can find another potential Group Owner in the vicinity
    private boolean mIsConnectedToGroup = false; // Determines whether the LEAVE button is enabled or not

    private Button mBtn_groupFormation;
    private Button mBtn_groupLeave;

    private Map<String, OpportunisticPeer> mPeers = new HashMap<>();
    private Map<String, NsdService> mServices = new HashMap<>();

    private WifiP2pPeerAdapter mWifiP2pPeerAdapter;
    private NsdServiceAdapter mNsdServiceAdapter;


    /** Fragment lifecycle method. See https://developer.android.com/guide/components/fragments.html
     * @param context Android-provided Application context
     */
    @Override
    public void onAttach(Context context) {
        super.onAttach(context);

        WifiP2pManager mWifiP2pManager = (WifiP2pManager) context.getSystemService(Context.WIFI_P2P_SERVICE);
        WifiP2pManager.Channel mWifiP2pChannel = mWifiP2pManager.initialize(context, Looper.getMainLooper(), null);

        mDiscoveryDetector = new DiscoveryDetector();

        IntentFilter intentFilter = new IntentFilter(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        intentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        context.registerReceiver(mDiscoveryDetector, intentFilter);

        mWifiP2pPeerAdapter = new WifiP2pPeerAdapter(context);
        mNsdServiceAdapter = new NsdServiceAdapter(context);

        mPeers.putAll(mWifiP2pPeerTracker.getPeers());
        mServices.putAll(mServiceTracker.getServices());

        mWifiP2pConnectivityManager.enable(context, mWifiP2pManager, mWifiP2pChannel, Utilities.obtainUuid(context));
    }

    /** Fragment lifecycle method (see https://developer.android.com/guide/components/fragments.html) */
    @Override
    public void onDetach() {
        mWifiP2pConnectivityManager.disable();
        getContext().unregisterReceiver(mDiscoveryDetector);
        super.onDetach();
    }

    /** Fragment lifecycle method. See https://developer.android.com/guide/components/fragments.html */
    @Override
    public void onResume() {
        super.onResume();
        mServiceTracker.addObserver(this);
        mWifiP2pPeerTracker.addObserver(this);
    }

    /** Fragment lifecycle method. See https://developer.android.com/guide/components/fragments.html */
    @Override
    public void onPause() {
        mServiceTracker.deleteObserver(this);
        mWifiP2pPeerTracker.deleteObserver(this);
        super.onPause();
    }

    /** Fragment lifecycle method. See https://developer.android.com/guide/components/fragments.html */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.v(TAG, "onCreateView");
        View viewWifiP2pTracking = inflater.inflate(R.layout.fragment_opp_peer_tracking, container, false);

        mDiscoveryInProgress = (ProgressBar) viewWifiP2pTracking.findViewById(R.id.discoveryInProgress);

        mBtn_groupFormation = (Button) viewWifiP2pTracking.findViewById(R.id.button_group_formation);
        mBtn_groupFormation.setEnabled(mCanFormGroup);
        mBtn_groupFormation.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mWifiP2pConnectivityManager.join(mWifiP2pPeerTracker.getPeers());
            }
        });

        mBtn_groupLeave = (Button) viewWifiP2pTracking.findViewById(R.id.button_group_leave);
        mBtn_groupLeave.setEnabled(mIsConnectedToGroup);
        mBtn_groupLeave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mWifiP2pConnectivityManager.leave();
            }
        });

        ListView listViewWifiP2pPeers = (ListView) viewWifiP2pTracking.findViewById(R.id.list_wifiP2pPeers);
        listViewWifiP2pPeers.setAdapter(mWifiP2pPeerAdapter);
        mWifiP2pPeerAdapter.clear();
        mWifiP2pPeerAdapter.addAll(mPeers.values());

        ListView listViewServices = (ListView) viewWifiP2pTracking.findViewById(R.id.list_nsd_services);
        listViewServices.setAdapter(mNsdServiceAdapter);
        mNsdServiceAdapter.clear();
        mNsdServiceAdapter.addAll(mServices.values());

        return viewWifiP2pTracking;
    }

    private Runnable mPeerUpdater = new Runnable() {
        @Override
        public void run() {
            mWifiP2pPeerAdapter.clear();
            Log.d(TAG, "New Peer list : " + mPeers.values());
            mWifiP2pPeerAdapter.addAll(mPeers.values());
        }
    };

    private Runnable mServiceUpdater = new Runnable() {
        @Override
        public void run() {
            mNsdServiceAdapter.clear();
            mNsdServiceAdapter.addAll(mServices.values());
        }
    };

    @Override
    public void update(Observable observable, Object obj) {
        FragmentActivity act = getActivity();

        if(observable instanceof OpportunisticPeerTracker) {
            /* When the PeerTracker notifies of some changes to its list, retrieve the new list of Peers
               and use it to update the UI accordingly. */
            mPeers.clear();
            mPeers.putAll(mWifiP2pPeerTracker.getPeers());

            mCanFormGroup = !mWifiP2pConnectivityManager.isAspiringGroupOwner(mPeers);
            mBtn_groupFormation.setEnabled(mCanFormGroup);

            if(act != null)
                act.runOnUiThread(mPeerUpdater);
        } else if (observable instanceof NsdServiceDiscoverer) {
            /* When the NSD Service Tracker notifies of some changes to its list, retrieve the new list of services
               and use it to update the UI accordingly. */
            if(obj != null) {
                NsdService svc = (NsdService) obj;
                mServices.put(svc.getUuid(), svc);
            } else {
                mServices.clear();
                mServices.putAll(mServiceTracker.getServices());
            }

            if(act != null)
                act.runOnUiThread(mServiceUpdater);
        }
    }

    // Used to toggle the visibility of the ProgressBar based on whether peer discovery is running
    private class DiscoveryDetector extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
                int extra = intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE, -1);
                if(extra == WifiP2pManager.WIFI_P2P_DISCOVERY_STARTED)
                    mDiscoveryInProgress.setVisibility(View.VISIBLE);
                else
                    mDiscoveryInProgress.setVisibility(View.INVISIBLE);


            } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
                NetworkInfo netInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
                boolean wifiP2pConnected = netInfo.isConnected();
                Log.v(TAG, "Connection changed : " + (wifiP2pConnected ? "CONNECTED" : "DISCONNECTED"));

                mIsConnectedToGroup = wifiP2pConnected;
                mBtn_groupLeave.setEnabled(mIsConnectedToGroup);
            }
        }
    }
}