#include "AttackPlayerBTTaskNode.h"

#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UAttackPlayerBTTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AIController->AttackPlayer();
	return EBTNodeResult::Succeeded;
}
