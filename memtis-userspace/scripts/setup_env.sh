#!/bin/bash
CONFIG_CXL_MODE=$1
DIR=/mnt/newdrive/memtis/memtis-userspace
function test_hello() {
    echo "111"
}
function func_memtis_setting() {
    echo 199 | sudo tee /sys/kernel/mm/htmm/htmm_sample_period
    echo 100007 | sudo tee /sys/kernel/mm/htmm/htmm_inst_sample_period
    echo 1 | sudo tee /sys/kernel/mm/htmm/htmm_thres_hot
    echo 2 | sudo tee /sys/kernel/mm/htmm/htmm_split_period
    echo 100000 | sudo tee /sys/kernel/mm/htmm/htmm_adaptation_period
    echo 2000000 | sudo tee /sys/kernel/mm/htmm/htmm_cooling_period
    echo 2 | sudo tee /sys/kernel/mm/htmm/htmm_mode
    echo 500 | sudo tee /sys/kernel/mm/htmm/htmm_demotion_period_in_ms
    echo 500 | sudo tee /sys/kernel/mm/htmm/htmm_promotion_period_in_ms
    echo 4 | sudo tee /sys/kernel/mm/htmm/htmm_gamma
    ###  cpu cap (per mille) for ksampled
    echo 30 | sudo tee /sys/kernel/mm/htmm/ksampled_soft_cpu_quota
    
    # if [[ "x${CONFIG_NS}" == "xoff" ]]; then
    #     echo 1 | sudo tee /sys/kernel/mm/htmm/htmm_thres_split
    # else
    #     echo 0 | sudo tee /sys/kernel/mm/htmm/htmm_thres_split
    # fi
    
    # if [[ "x${CONFIG_NW}" == "xoff" ]]; then
    #     echo 0 | sudo tee /sys/kernel/mm/htmm/htmm_nowarm
    # else
    #     echo 1 | sudo tee /sys/kernel/mm/htmm/htmm_nowarm
    # fi
    
    if [[ "x${CONFIG_CXL_MODE}" == "xon" ]]; then
        ${DIR}/scripts/set_uncore_freq.sh on
        echo "enabled" | sudo tee /sys/kernel/mm/htmm/htmm_cxl_mode
    else
        ${DIR}/scripts/set_uncore_freq.sh off
        echo "disabled" | sudo tee /sys/kernel/mm/htmm/htmm_cxl_mode
    fi
    
    echo "always" | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
    echo "always" | sudo tee /sys/kernel/mm/transparent_hugepage/defrag
}

# Check if the script is being sourced or executed directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    func_memtis_setting
fi