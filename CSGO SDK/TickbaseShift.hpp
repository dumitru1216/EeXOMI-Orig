#pragma once
#include "sdk.hpp"



struct local_data_t {

	float					m_spawn_time{}, m_abs_yaw{};

	int						m_tick_base{},
		m_adjusted_tick_base{}, m_shift_amount{};
	bool					m_override_tick_base{}, m_restore_tick_base{};

	Vector					m_move{};
	CUserCmd		m_user_cmd{};
};

class c_exploits {
public:


	bool is_charged( );
	void cl_move( bool bFinalTick, float accumulated_extra_samples );
	void handle_exploits( bool* bSendPacket, CUserCmd* pCmd );

	void skip_lag_interpolation( bool targeting );

	// charge related stuff
	bool m_is_charged = false;
	int m_charged_ticks = 0;
	int m_max_process_ticks = 14;

	// is this the 2nd dt shot?
	bool m_second_shot = false;

	// are we shifting rn?
	bool b_shifting = false;

	// used for WriteUsercmdDeltaToBuffer shifting
	int m_shift_amount = 0;
	bool m_can_shift = false;
	bool m_break_lc = false;
	int m_break_lc_cycle = 0;

	int m_shift_to_fix;
	bool m_should_fix = false;;

	float m_last_exploit_time = FLT_MAX;
	int m_last_shift_time = FLT_MAX;
	int m_shift_command = 0;

	bool m_should_charge = false;
	bool m_double_tap = false;


	struct cfg_t {
		bool	m_enabled{}, m_defensive_dt{};
		int		m_additions{}, m_dt_key{}, m_hide_shots_key{}, m_break_lc_timer{}, m_defensive_shift{ 14 };
	};

	bool					m_charged{}, m_shift{},
		m_force_choke{}, m_dt_ready{}, m_using_dt_hc{};
	int						m_ticks_allowed{}, m_cur_shift_amount{},
		m_next_shift_amount{}, m_recharge_cmd{}, m_type{}, m_correction_amount{}, m_break_lagcomp_ticks{}, m_max_recharge{ 14 }, m_predicted_tick{};

	bool m_charging{};
	float m_last_shot{};
	bool m_can_choke{};
	bool m_can_shoot{};
	bool m_choke{};

	int calc_correction_ticks( ) const;

	int adjust_tick_base( const int old_new_cmds, const int total_new_cmds, const int delta ) const;



	std::array< local_data_t, 150u >	m_local_data{};
	void handle_break_lc( void* ecx, void* edx, const int slot, bf_write* buffer, int& from, int& to, int* m_new_cmds, int* m_backup_cmds );
	void handle_other_shift( void* ecx, void* edx, const int slot, bf_write* buffer, int& from, int& to, int* m_new_cmds, int* m_backup_cmds );
};

extern c_exploits g_tickbase_control;