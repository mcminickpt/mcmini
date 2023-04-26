#include "mcmini/transitions/misc/MCBankAccountIncrement.h"
#include "mcmini/transitions/misc/MCBankAccountDecrement.h"

MCTransition *
MCReadBankAccountIncrement(const MCSharedTransition *shmTransition,
                           void *shmStart, MCState *state)
{
  auto data     = *(MCBankAccountShadow *)shmStart;
  auto executor = state->getThreadWithId(shmTransition->executor);
  auto bankAccount =
    state->getVisibleObjectWithSystemIdentity<MCBankAccount>(
      data.identity);

  /* New global variable */
  if (bankAccount == nullptr) {
    bankAccount = std::make_shared<MCBankAccount>(data);
    state->registerVisibleObjectWithSystemIdentity(data.identity,
                                                   bankAccount);
  }

  return new MCBankAccountIncrement(std::move(executor),
                                    std::move(bankAccount));
}

std::shared_ptr<MCTransition>
MCBankAccountIncrement::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto bankCpy =
    std::static_pointer_cast<MCBankAccount, MCVisibleObject>(
      this->bank->copy());
  return std::make_shared<MCBankAccountIncrement>(
    std::move(threadCpy), std::move(bankCpy));
}

std::shared_ptr<MCTransition>
MCBankAccountIncrement::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  auto bankAccountInState =
    state->getObjectWithId<MCBankAccount>(bank->getObjectId());
  return std::make_shared<MCBankAccountIncrement>(
    std::move(threadInState), std::move(bankAccountInState));
}

bool
MCBankAccountIncrement::dependentWith(
  const MCTransition *transition) const
{
  // Dependent with increments and decrements alike
  {
    const MCBankAccountIncrement *increment =
      dynamic_cast<const MCBankAccountIncrement *>(transition);
    if (increment != nullptr) {
      return *this->bank == *increment->bank;
    }
  }

  {
    const MCBankAccountDecrement *decrement =
      dynamic_cast<const MCBankAccountDecrement *>(transition);
    if (decrement != nullptr) {
      return *this->bank == *decrement->bank;
    }
  }

  // Independent with everything else
  return false;
}

void
MCBankAccountIncrement::applyToState(MCState *state)
{
  this->bank->increment(this->amount);
}

void
MCBankAccountIncrement::print() const
{
  printf("thread %lu: increment_account(by %u)\n", this->thread->tid,
         this->amount);
}
