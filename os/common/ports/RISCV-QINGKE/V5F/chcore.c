/*
    ChibiOS - Copyright (C) 2006-2026 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    ...

/**
 * @file    V5F/chcore_v5f.c
 * @brief   SKELETON - V5F port related code.
 *
 * @addtogroup port_core
 * @{
 */

#include "ch.h"

/* TODO: Implement V5F context switching, IRQ prologue/epilogue */

void _port_init(void) {}

__attribute__((naked)) void _port_switch(thread_t *ntp, thread_t *otp) {
  (void)ntp; (void)otp;
  /* TODO: Implement V5F context switch */
  while (1);
}

__attribute__((naked, section(".irq_entry"))) void _port_irq_handler(void) {
  /* TODO: Implement V5F IRQ handler */
  while (1);
}

__attribute__((naked)) void _port_thread_start() {
  /* TODO: Implement V5F thread start */
  while (1);
}

/** @} */
